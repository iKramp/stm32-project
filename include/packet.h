#ifndef COMMON_PACKET_H
#define COMMON_PACKET_H

#include <stdint.h>

uint8_t send_udp(uint8_t *data, uint16_t length, uint8_t *dest_ip, uint16_t dest_port);
void process_incoming_packets();


#endif // COMMON_PACKET_H
