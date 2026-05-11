#ifndef SERVER_HPP
#define SERVER_HPP
#include <cstdint>

enum RegionState {
    NOT_SENT,
    SENT,
    FINISHED,
};

enum JobType {
    NONE,
    SEND_SCENE_DATA,
    SEND_REGION_DATA,
};

struct SendSceneDataJob {
    uint32_t current_offset;
    uint32_t length;
};

struct ClientJob {
    enum JobType type;
    union {
        struct SendSceneDataJob send_scene_data_job;
    } payload;
};

struct ClientData {
    uint32_t last_active; //timestamp of last activity
    uint32_t ping_time; //timestamp of last ping, used to check if client is still alive
    struct ClientJob current_job;
    uint16_t port;
    uint8_t valid;
    uint8_t ip[4];
    uint8_t mac[6];
    uint8_t rendering;
    uint8_t region_x;
    uint8_t region_y;
};

void server_main();

#endif // SERVER_HPP
