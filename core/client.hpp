#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <cstdint>
#include "ray_tracer/camdata.hpp"

enum StateEnum {
    NOT_CONNECTED,
    GETTING_SCENE,
    GETTING_CAMDATA,
    RENDERING,
    SENDING_RENDER,
};

struct DataReceptionState {
    uint32_t total_length;
    uint32_t received_length;
    uint32_t last_requested_timestamp;
};

struct CamDataReceptionState {
    uint32_t last_requested_timestamp;
};

struct RenderState {
    uint32_t x;
    uint32_t y;
};

struct State {
    enum StateEnum state;
    uint32_t last_packet_timestamp;
    uint8_t *data_buffer;
    struct CamData cam_data;
    union {
        struct DataReceptionState data_reception_state;
        struct CamDataReceptionState camdata_reception_state;
        struct RenderState render_state;
    } data;
};

void client_main();

#endif // CLIENT_HPP
