#ifndef MESSAGE_H
#define MESSAGE_H

#include "ray_tracer_types.hpp"
#include <stdint.h>

enum MessageType {
    PING = 0,
    PONG = 1,
    REQUEST_DATA = 2,
    REQUEST_CAMDATA = 3,
    DATA_RESPONSE = 4,
    CAMDATA_RESPONSE = 5,
    DATA_FINISHED = 6,
    HELLO = 7,
    NUM_MESSAGE_TYPES
};

struct PingMessage {};
struct PongMessage {};
struct RequestDataMessage {};
struct RequestCamdataMessage {};
struct DataResponseMessage {
    uint32_t offset;
    uint32_t length;
    uint32_t total_length;
    uint8_t data[512]; //leave room for network headers
};
//C compat camdata
struct CamdataResponseMessage {
    CamData cam_data;
};
struct DataFinishedMessage {
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
    uint32_t offset;
    uint32_t length;
    uint8_t data[512]; //leave room for network headers
};
struct HelloMessage {};

struct Message {
    uint32_t magic; //0xDEADBEEF
    uint32_t type;
    union {
        struct PingMessage ping;
        struct PongMessage pong;
        struct RequestDataMessage request_data;
        struct RequestCamdataMessage request_region;
        struct DataResponseMessage data_response;
        struct CamdataResponseMessage region_response;
        struct DataFinishedMessage data_finished;
        struct HelloMessage hello;
    } payload;
};

uint32_t get_message_size(struct Message *msg);


#endif // MESSAGE_H
