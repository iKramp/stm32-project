#include "packet_handler.hpp"

//C compat wrapper
#include <stdio.h>
#include <string.h>
extern "C" {
    #include "handle_packet.h"
    #include "../../include/packet.h"
}

//arr of void functions for handling different message types
MessageHandler message_handlers[NUM_MESSAGE_TYPES] = {0};

void register_message_handler(enum MessageType type, MessageHandler handler) {
    message_handlers[type] = handler;
}

MessageHandler get_message_handler(uint32_t type) {
    return message_handlers[type];
}

void ping_handler(struct Message *msg, uint8_t *src_ip, uint16_t src_port) {
    struct Message reply = {0};
    reply.magic = 0xDEADBEEF;
    reply.type = PONG;

    uint32_t msg_size = get_message_size(&reply);

    send_udp((uint8_t *)&reply, msg_size, src_ip, src_port);
}

void init_packet_handlers(uint8_t server) {
    register_message_handler(PING, ping_handler);
}

//starts with ethernet header
void handle_udp_packet(uint8_t *packet, uint32_t length, uint8_t *src_ip, uint16_t src_port) {
    struct Message *msg = (struct Message *)packet;
    if (msg->magic != 0xDEADBEEF) {
        return; //not a valid message
    }

    MessageHandler handler = get_message_handler(msg->type);
    if (handler) {
        handler(msg, src_ip, src_port);
    } else {
        printf("No handler for message type %d\n", msg->type);
    }
}
