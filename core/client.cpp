#include "client.hpp"
#include "net/message.hpp"
#include "net/packet_handler.hpp"
#include "ray_tracer/mod.hpp"
#include <string.h>
extern "C" {
    #include "scene_data.h"
    #include "../include/clock.h"
    #include "../include/draw.h"
    #include "../include/panic.h"
    #include "../include/packet.h"
    #include "../include/platform_specific.h"
}

#include "../include/settings.h"
#include <stdio.h>

static State state = { .state = NOT_CONNECTED };
static uint8_t *pixel_buffer = 0;

uint8_t calculate_hash_client(uint8_t *data, uint32_t length) {
    //simple xor hash for testing
    uint8_t hash = 0;
    for (uint32_t i = 0; i < length; i++) {
        hash ^= data[i];
    }
    return hash;
}

void handle_data_response(struct Message *msg, uint8_t *src_ip, uint16_t src_port) {
    uint32_t expected_offset = state.data.data_reception_state.received_length;
    if (msg->payload.data_response.offset != expected_offset) {
        //ignore
        return;
    }

    state.last_packet_timestamp = get_time();
    state.data.data_reception_state.total_length = msg->payload.data_response.total_length;
    //copy
    uint32_t copy_length = msg->payload.data_response.length;
    if (copy_length > 512) {
        panic("Received data response with length greater than 512");
    }
    if (state.data.data_reception_state.received_length + copy_length > state.data.data_reception_state.total_length) {
        panic("Received data response that exceeds total length");
    }
    memcpy(state.data_buffer + state.data.data_reception_state.received_length, msg->payload.data_response.data, copy_length);
    state.data.data_reception_state.received_length += copy_length;
    if (state.data.data_reception_state.received_length == state.data.data_reception_state.total_length) {
        printf("Received all data, total length: %u bytes\n", state.data.data_reception_state.total_length);
        printf("Data hash: 0x%02X\n", calculate_hash_client(state.data_buffer, state.data.data_reception_state.total_length));
        state.state = GETTING_CAMDATA;
        state.data.camdata_reception_state.last_requested_timestamp = 0;
        parse_scene_data(state.data_buffer);
        set_scene_data(state.data_buffer, state.data.data_reception_state.total_length);
        pixel_buffer = state.data_buffer + state.data.data_reception_state.total_length;
    }
}

void handle_camdata_response(struct Message *msg, uint8_t *src_ip, uint16_t src_port) {
    state.last_packet_timestamp = get_time();
    state.cam_data = msg->payload.region_response.cam_data;
    printf("Received camdata response\n");

    printf("Rendering region (%u, %u) with size (%u, %u)\n", state.cam_data.top_left_x, state.cam_data.top_left_y, state.cam_data.region_width, state.cam_data.region_height);

    state.state = RENDERING;
    state.data.render_state.x = 0;
    state.data.render_state.y = 0;
}

void handle_data_finished_response(struct Message *msg, uint8_t *src_ip, uint16_t src_port) {
    if (msg->payload.data_finished_response.success) {
        printf("Server successfully received rendered data for region (%u, %u)\n", state.cam_data.top_left_x, state.cam_data.top_left_y);
        state.state = GETTING_CAMDATA;
        state.data.camdata_reception_state.last_requested_timestamp = 0;
        state.last_packet_timestamp = get_time(); //prevent timeout
    } else {
        printf("Server reported error receiving rendered data for region (%u, %u), expected offset: %u\n", state.cam_data.top_left_x, state.cam_data.top_left_y, msg->payload.data_finished_response.expected_offset);
        //resend from expected offset
        state.state = SENDING_RENDER;
        state.data.sending_render_state.offset = msg->payload.data_finished_response.expected_offset;
    }
}

void request_data(DataReceptionState *state) {
    Message msg = {};
    msg.magic = 0xDEADBEEF;
    msg.type = REQUEST_DATA;
    msg.payload.request_data.offset = state->received_length;

    uint32_t msg_size = get_message_size(&msg);

    send_udp((uint8_t *)&msg, msg_size, (uint8_t[])SERVER_IP_BYTES, PORT);
    state->last_requested_timestamp = get_time();
    printf("Requested data from offset %u\n", state->received_length);
}

void client_main() {
    state.data_buffer = get_data_buffer();
    register_message_handler(DATA_RESPONSE, handle_data_response);
    register_message_handler(CAMDATA_RESPONSE, handle_camdata_response);
    register_message_handler(DATA_FINISHED_RESPONSE, handle_data_finished_response);

    printf("Client started\n");

    while (1) {
        process_incoming_packets();

        switch (state.state) {
            case NOT_CONNECTED: {
                Message msg = {};
                msg.magic = 0xDEADBEEF;
                msg.type = HELLO;

                uint32_t msg_size = get_message_size(&msg);

                send_udp((uint8_t *)&msg, msg_size, (uint8_t[])SERVER_IP_BYTES, PORT);
                state.state = GETTING_SCENE;
                state.data.data_reception_state.total_length = 0;
                state.data.data_reception_state.received_length = 0;
                state.last_packet_timestamp = 0;
                state.data.data_reception_state.last_requested_timestamp = 0;

                break;
            };
            case GETTING_SCENE: {
                if (state.data.data_reception_state.last_requested_timestamp == 0) {
                    //just so we don't timeout immediately
                    state.last_packet_timestamp = get_time();
                    request_data(&state.data.data_reception_state);
                    break;
                }

                if (state.last_packet_timestamp + 500 < get_time() && state.data.data_reception_state.last_requested_timestamp + 500 < get_time()) {
                    //no data received for 500ms, request again
                    request_data(&state.data.data_reception_state);
                }

                if (state.last_packet_timestamp + 5000 < get_time()) {
                    panic("No data received for 5 seconds, giving up");
                }
                break;
            };
            case GETTING_CAMDATA: {
                if (state.data.camdata_reception_state.last_requested_timestamp + 500 < get_time()) {
                    //request camdata again
                    Message msg = {};
                    msg.magic = 0xDEADBEEF;
                    msg.type = REQUEST_CAMDATA;

                    uint32_t msg_size = get_message_size(&msg);

                    send_udp((uint8_t *)&msg, msg_size, (uint8_t[])SERVER_IP_BYTES, PORT);
                    state.data.camdata_reception_state.last_requested_timestamp = get_time();
                }

                if (state.last_packet_timestamp + 5000 < get_time()) {
                    panic("No camdata received for 5 seconds, giving up");
                }

                break;
            };
            case RENDERING: {
                uint32_t x = state.data.render_state.x;
                uint32_t y = state.data.render_state.y;
                uint32_t tracer_x = x + state.cam_data.top_left_x;
                uint32_t tracer_y = y + state.cam_data.top_left_y;
                printf("Rendering pixel (%u, %u)\n", tracer_x, tracer_y);
                uint32_t color = tracer_main(state.cam_data, tracer_x, tracer_y);

                //write color to framebuffer
                uint32_t fb_width;
                uint32_t fb_height;
                get_fb_dimensions(&fb_width, &fb_height);
                uint32_t draw_x = x + fb_width / 2;
                uint32_t draw_y = y;
                draw_pixel(draw_x, draw_y, color);

                uint32_t offset = (y * state.cam_data.region_width + x) * 4;
                if (offset + 4 > state.cam_data.region_width * state.cam_data.region_height * 4) {
                    break; //do nothing
                }
                memcpy(pixel_buffer + offset, &color, 4);

                state.data.render_state.x++;
                if (state.data.render_state.x >= state.cam_data.region_width) {
                    state.data.render_state.x = 0;
                    state.data.render_state.y++;
                    if (state.data.render_state.y >= state.cam_data.region_height) {
                        printf("Finished rendering region (%u, %u)\n", state.cam_data.top_left_x, state.cam_data.top_left_y);
                        state.state = SENDING_RENDER;
                        state.data.sending_render_state.offset = 0;
                    }
                }

                break;
            };
            case SENDING_RENDER: {
                if (state.data.sending_render_state.offset >= state.cam_data.region_width * state.cam_data.region_height * 4) {
                    break; //nothing to send
                }

                Message msg = {};
                msg.magic = 0xDEADBEEF;
                msg.type = DATA_FINISHED;
                msg.payload.data_finished.x = state.cam_data.top_left_x;
                msg.payload.data_finished.y = state.cam_data.top_left_y;
                msg.payload.data_finished.width = state.cam_data.region_width;
                msg.payload.data_finished.height = state.cam_data.region_height;
                msg.payload.data_finished.offset = state.data.sending_render_state.offset;
                uint32_t remaining_bytes = state.cam_data.region_width * state.cam_data.region_height * 4 - state.data.sending_render_state.offset;
                uint32_t chunk_size = remaining_bytes > 512 ? 512 : remaining_bytes;
                msg.payload.data_finished.length = chunk_size;
                memcpy(msg.payload.data_finished.data, pixel_buffer + state.data.sending_render_state.offset, chunk_size);

                uint32_t msg_size = get_message_size(&msg);
                send_udp((uint8_t *)&msg, msg_size, (uint8_t[])SERVER_IP_BYTES, PORT);
                state.data.sending_render_state.offset += chunk_size;
            }
        }
    }
}
