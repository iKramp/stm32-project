#ifndef HANDLE_PACKET_H
#define HANDLE_PACKET_H

//C compat function
#include <stdint.h>
void handle_udp_packet(uint8_t *packet, uint32_t length, uint8_t *src_ip, uint16_t src_port);

#endif // HANDLE_PACKET_H
