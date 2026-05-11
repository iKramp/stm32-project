#ifndef PACKET_HANDLER_H
#define PACKET_HANDLER_H
#include <stdint.h>
#include "message.hpp"

typedef void (*MessageHandler)(struct Message *msg, uint8_t *src_ip, uint8_t *src_mac, uint16_t src_port);

void init_packet_handlers(uint8_t server);
void register_message_handler(enum MessageType type, MessageHandler handler);

#endif // PACKET_HANDLER_H
