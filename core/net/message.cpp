#include "message.hpp"
extern "C" {
    #include "../../include/panic.h"
}

uint32_t get_inner_message_size(struct Message *msg) {
    switch (msg->type) {
        case PING:
            return sizeof(struct PingMessage);
        case PONG:
            return sizeof(struct PongMessage);
        case REQUEST_DATA:
            return sizeof(struct RequestDataMessage);
        case REQUEST_CAMDATA:
            return sizeof(struct RequestCamdataMessage);
        case DATA_RESPONSE: {
            uint32_t data_length = msg->payload.data_response.length;
            if (data_length > 512) {
                panic("data length too large");
            }
            return sizeof(struct DataResponseMessage) - 512 + data_length;
        }
        case CAMDATA_RESPONSE:
            return sizeof(struct CamdataResponseMessage);
        case DATA_FINISHED: {
            uint32_t data_length = msg->payload.data_finished.length;
            if (data_length > 512) {
                panic("data length too large");
            }
            return sizeof(struct DataFinishedMessage) - 512 + data_length;
        }
        case DATA_FINISHED_RESPONSE:
            return sizeof(struct DataFinishedResponseMessage);
        case HELLO:
            return sizeof(struct HelloMessage);
        default:
            panic("invalid message type");
    }
    return 0; //unreachable
}

uint32_t get_message_size(struct Message *msg) {
    return get_inner_message_size(msg) + sizeof(msg->magic) + sizeof(msg->type);
}
