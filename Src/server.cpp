#include "server.hpp"
#include "peripherals/nvic.h"
#include "ray_tracer/camdata.hpp"
#include "ray_tracer/mod.hpp"
#include "net/message.hpp"
#include "net/packet_handler.hpp"
extern "C" {
    #include "rendering/framebuffer.h"
    #include "hal/common.h"
    #include "peripherals/eth/eth_transmit.h"
    #include "peripherals/clock.h"
}

#include <cstring>
#include <stdio.h>

static struct ClientData clients[10];
static enum RegionState regions[10][10];

void handle_hello(struct Message *msg, uint8_t *src_ip, uint8_t *src_mac, uint16_t src_port) {
    //check if client already exists
    for (auto &client : clients) {
        if (client.valid && memcmp(client.ip, src_ip, 4) == 0 && memcmp(client.mac, src_mac, 6) == 0 && client.port == src_port) {
            return; //client already exists
        }
    }

    //add new client
    for (auto &client : clients) {
        if (!client.valid) {
            memcpy(client.ip, src_ip, 4);
            memcpy(client.mac, src_mac, 6);
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

void handle_pong(struct Message *msg, uint8_t *src_ip, uint8_t *src_mac, uint16_t src_port) {
    //update client last active time
    for (auto &client : clients) {
        if (client.valid && memcmp(client.ip, src_ip, 4) == 0 && memcmp(client.mac, src_mac, 6) == 0 && client.port == src_port) {
            client.last_active = get_time();
            return;
        }
    }
}

void handle_request_data(struct Message *msg, uint8_t *src_ip, uint8_t *src_mac, uint16_t src_port) {
    //find client and set job to send scene data
    for (auto &client : clients) {
        if (client.valid && memcmp(client.ip, src_ip, 4) == 0 && memcmp(client.mac, src_mac, 6) == 0 && client.port == src_port) {
            client.current_job.type = SEND_SCENE_DATA;
            client.current_job.payload.send_scene_data_job.current_offset = 0;
            printf("Received data request from client: %d.%d.%d.%d:%d\n", src_ip[0], src_ip[1], src_ip[2], src_ip[3], src_port);
            return;
        }
    }
}

void handle_request_region(struct Message *msg, uint8_t *src_ip, uint8_t *src_mac, uint16_t src_port) {
    //find client and set job to send region data
    for (auto &client : clients) {
        if (client.valid && memcmp(client.ip, src_ip, 4) == 0 && memcmp(client.mac, src_mac, 6) == 0 && client.port == src_port) {
            CamData cam_data = get_cam_data();
            struct FrameBuffer *fb = get_fb();
            cam_data.canvas_width = fb->width;
            cam_data.canvas_height = fb->height;
            //find region to send
            uint8_t region_x = -1;
            uint8_t region_y = -1;
            for (int y = 0; y < 10; y++) {
                for (int x = 0; x < 10; x++) {
                    if (regions[y][x] == NOT_SENT) {
                        regions[y][x] = SENT;
                        region_x = x;
                        region_y = y;
                        break;
                    }
                }
            }
            if (region_x == (uint8_t)-1 || region_y == (uint8_t)-1) {
                printf("No regions available to send to client: %d.%d.%d.%d:%d\n", src_ip[0], src_ip[1], src_ip[2], src_ip[3], src_port);
                return; //no regions available
            }
            client.region_x = region_x;
            client.region_y = region_y;
            client.rendering = 1;
            
            cam_data.top_left_x = region_x * (fb->width / 10);
            cam_data.top_left_y = region_y * (fb->height / 10);
            cam_data.region_width = fb->width / 10;
            cam_data.region_height = fb->height / 10;

            struct Message network_msg = {};
            network_msg.magic = 0xDEADBEEF;
            network_msg.type = CAMDATA_RESPONSE;
            network_msg.payload.region_response.cam_data = cam_data;

            uint32_t msg_size = get_message_size(&network_msg);
            uint8_t success = transmit_data_udp((uint8_t *)&network_msg, msg_size, client.ip, client.mac, client.port);
            if (!success) {
                printf("Failed to send camdata response to client: %d.%d.%d.%d:%d\n", src_ip[0], src_ip[1], src_ip[2], src_ip[3], src_port);
                regions[region_y][region_x] = NOT_SENT; //mark region as not sent so it can be retried later
                return;
            }
        }
    }
}

void handle_data_finished(struct Message *msg, uint8_t *src_ip, uint8_t *src_mac, uint16_t src_port) {
    //handle data finished message
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
    uint8_t success = transmit_data_udp((uint8_t *)&network_msg, sizeof(network_msg), client->ip, client->mac, client->port);
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

        transmit_data_udp((uint8_t *)&msg, msg_size, client->ip, client->mac, client->port);
        client->ping_time = get_time();
    }
}

void server_main() {
    register_message_handler(HELLO, handle_hello);
    register_message_handler(PONG, handle_pong);
    register_message_handler(REQUEST_DATA, handle_request_data);

    printf("Server main started\n");
    while (1) {
        for (auto &client : clients) {
            if (client.valid) {
                uint8_t before = irq_save();
                handle_client(&client);
                irq_restore(before);
            }
        }
    }
}
