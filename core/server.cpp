#include "server.hpp"
#include "ray_tracer/camdata.hpp"
#include "net/message.hpp"
#include "net/packet_handler.hpp"
extern "C" {
    #include "../include/interrupt.h"
    #include "../include/draw.h"
    #include "../include/clock.h"
    #include "../include/panic.h"
    #include "../include/packet.h"
    #include "scene_data.h"
}

#include <cstring>
#include <stdio.h>

static struct ClientData clients[10];
static enum RegionState regions[10][10];

uint8_t calculate_hash_server(uint8_t *data, uint32_t length) {
    //simple xor hash for testing
    uint8_t hash = 0;
    for (uint32_t i = 0; i < length; i++) {
        hash ^= data[i];
    }
    return hash;
}

void handle_hello(struct Message *msg, uint8_t *src_ip, uint16_t src_port) {
    //check if client already exists
    for (auto &client : clients) {
        if (client.valid && memcmp(client.ip, src_ip, 4) == 0 && client.port == src_port) {
            return; //client already exists
        }
    }

    //add new client
    for (auto &client : clients) {
        if (!client.valid) {
            memcpy(client.ip, src_ip, 4);
            client.port = src_port;
            client.valid = 1;
            client.last_active = get_time();
            client.current_job.type = NONE;
            printf("New client connected: %d.%d.%d.%d:%d\n", src_ip[0], src_ip[1], src_ip[2], src_ip[3], src_port);
            return;
        }
    }

    printf("Max clients reached, rejecting new client\n");
}

void handle_pong(struct Message *msg, uint8_t *src_ip, uint16_t src_port) {
    //update client last active time
    for (auto &client : clients) {
        if (client.valid && memcmp(client.ip, src_ip, 4) == 0 && client.port == src_port) {
            client.last_active = get_time();
            return;
        }
    }
}

void handle_request_data(struct Message *msg, uint8_t *src_ip, uint16_t src_port) {
    //find client and set job to send scene data
    for (auto &client : clients) {
        if (client.valid && memcmp(client.ip, src_ip, 4) == 0 && client.port == src_port) {
            client.current_job.type = SEND_SCENE_DATA;
            client.current_job.payload.send_scene_data_job.current_offset = msg->payload.request_data.offset;
            printf("Received data request from client: %d.%d.%d.%d:%d\n", src_ip[0], src_ip[1], src_ip[2], src_ip[3], src_port);
            return;
        }
    }
}

void handle_request_camdata(struct Message *msg, uint8_t *src_ip, uint16_t src_port) {
    //find client and set job to send region data
    for (auto &client : clients) {
        if (client.valid && memcmp(client.ip, src_ip, 4) == 0 && client.port == src_port) {
            CamData cam_data = get_cam_data();
            uint32_t fb_width, fb_height;
            get_fb_dimensions(&fb_width, &fb_height);
            cam_data.canvas_height = fb_height;
            cam_data.canvas_width = fb_width;
            //find region to send
            uint8_t region_x = -1;
            uint8_t region_y = -1;
            for (int y = 0; y < 10; y++) {
                for (int x = 0; x < 10; x++) {
                    if (regions[y][x] == NOT_SENT) {
                        regions[y][x] = SENT;
                        region_x = x;
                        region_y = y;
                        goto region_found;
                    }
                }
            }
            region_found:

            if (region_x == (uint8_t)-1 || region_y == (uint8_t)-1) {
                printf("No regions available to send to client: %d.%d.%d.%d:%d\n", src_ip[0], src_ip[1], src_ip[2], src_ip[3], src_port);
                return; //no regions available
            }
            client.region_x = region_x;
            client.region_y = region_y;
            client.rendering = 1;
            
            cam_data.top_left_x = region_x * (fb_width / 10);
            cam_data.top_left_y = region_y * (fb_height / 10);
            cam_data.region_width = fb_width / 10;
            cam_data.region_height = fb_height / 10;

            struct Message network_msg = {};
            network_msg.magic = 0xDEADBEEF;
            network_msg.type = CAMDATA_RESPONSE;
            network_msg.payload.region_response.cam_data = cam_data;

            printf("sending region (%d, %d) with size (%d, %d)\n", region_x, region_y, cam_data.region_width, cam_data.region_height);

            uint32_t msg_size = get_message_size(&network_msg);
            uint8_t success = send_udp((uint8_t *)&network_msg, msg_size, client.ip, client.port);
            if (!success) {
                printf("Failed to send camdata response to client: %d.%d.%d.%d:%d\n", src_ip[0], src_ip[1], src_ip[2], src_ip[3], src_port);
                regions[region_y][region_x] = NOT_SENT; //mark region as not sent so it can be retried later
                return;
            }
        }
    }
}

void handle_data_finished(struct Message *msg, uint8_t *src_ip, uint16_t src_port) {
    printf("Received data finished message from client: %d.%d.%d.%d:%d\n", src_ip[0], src_ip[1], src_ip[2], src_ip[3], src_port);
     //find client and mark region as finished
    for (auto &client : clients) {
        if (client.valid && memcmp(client.ip, src_ip, 4) == 0 && client.port == src_port) {
            uint8_t region_x = client.region_x;
            uint8_t region_y = client.region_y;
            if (region_x < 10 && region_y < 10) {
                regions[region_y][region_x] = FINISHED;
                printf("Marked region (%d, %d) as finished\n", region_x, region_y);
            }
            client.rendering = 0;
            return;
        }
    }
}

uint8_t send_scene_data(struct ClientData *client, struct SendSceneDataJob *job) {
    uint8_t *scene_data = get_scene_data();
    uint32_t scene_data_size = get_scene_data_size();
    uint32_t chunk_size = 512; //max payload size

    uint32_t remaining = scene_data_size - job->current_offset;
    if (remaining == 0) {
        return 0; //no more data to send
    }

    if (job->current_offset % chunk_size != 0) {
        panic("macro you idiot");
    }

    if (remaining < chunk_size) {
        chunk_size = remaining;
    }
    
    struct DataResponseMessage msg;
    msg.offset = job->current_offset;
    msg.length = chunk_size;
    msg.total_length = scene_data_size;
    memcpy(msg.data, scene_data + job->current_offset, chunk_size);

    struct Message network_msg = {};
    network_msg.magic = 0xDEADBEEF;
    network_msg.type = DATA_RESPONSE;
    network_msg.payload.data_response = msg;

    uint32_t msg_size = get_message_size(&network_msg);

    //send msg to client
    uint8_t success = send_udp((uint8_t *)&network_msg, sizeof(network_msg), client->ip, client->port);
    if (!success) {
        return 1; //retry later
    }

    job->current_offset += chunk_size;
    return 1; //more to do
}

void handle_client(struct ClientData *client) {
    //handle client job
    switch (client->current_job.type) {
        case NONE: {
            //no job, do nothing
            break;
        }
        case SEND_SCENE_DATA: {
            uint8_t more_to_do = send_scene_data(client, &client->current_job.payload.send_scene_data_job);
            if (!more_to_do) {
                client->current_job.type = NONE; //job finished
            }
            break;
        }
        case SEND_REGION_DATA: {
            //send some data
            break;
        }
    }

    if (get_time() - client->last_active > 10000) { //5 seconds timeout
        client->valid = 0; //remove client
        printf("Client timed out: %d.%d.%d.%d:%d\n", client->ip[0], client->ip[1], client->ip[2], client->ip[3], client->port);
        return;
    }
    if (get_time() - client->ping_time > 5000) { //5 seconds since last ping
        //send ping
        struct Message msg = {};
        msg.magic = 0xDEADBEEF;
        msg.type = PING;

        uint32_t msg_size = get_message_size(&msg);

        send_udp((uint8_t *)&msg, msg_size, client->ip, client->port);
        client->ping_time = get_time();
    }
}

void server_main() {
    register_message_handler(HELLO, handle_hello);
    register_message_handler(PONG, handle_pong);
    register_message_handler(REQUEST_DATA, handle_request_data);
    register_message_handler(REQUEST_CAMDATA, handle_request_camdata);
    register_message_handler(DATA_FINISHED, handle_data_finished);

    uint8_t *scene_data = get_scene_data();
    uint32_t scene_data_size = get_scene_data_size();
    uint8_t scene_data_hash = calculate_hash_server(scene_data, scene_data_size);
    printf("Scene data size: %u bytes\n", scene_data_size);
    printf("Scene data hash: 0x%02X\n", scene_data_hash);

    //initialize regions state
    for (int y = 0; y < 10; y++) {
        for (int x = 0; x < 10; x++) {
            regions[y][x] = NOT_SENT;
        }
    }

    printf("Server main started\n");
    while (1) {
        for (auto &client : clients) {
            if (client.valid) {
                uint32_t before = disable_interrupts();
                handle_client(&client);
                restore_interrupts(before);
            }
        }
        process_incoming_packets();
    }
}
