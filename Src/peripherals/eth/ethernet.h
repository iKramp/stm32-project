#ifndef ETH_H
#define ETH_H

#define ETHERNET_BASE 0x40028000

#include <stdint.h>
#include "../clock.h"
#include "../../hal/gpio.h"
#include "../../hal/register.h"
#include "../fmc.h"

#define NUM_DESCRIPTORS 128
#define UDP_PORT 12345
#define BROADCAST_MAC {0xff,0xff,0xff,0xff,0xff,0xff}
#define is_broadcast(ip, mask) ( \
    ((ip[0] & !mask[0]) == !mask[0]) && \
    ((ip[1] & !mask[1]) == !mask[1]) && \
    ((ip[2] & !mask[2]) == !mask[2]) && \
    ((ip[3] & !mask[3]) == !mask[3]) \
)

static inline uint8_t *make_broadcast_ip(uint8_t *ip, uint8_t *mask) {
    static uint8_t broadcast_ip[4];
    for (int i = 0; i < 4; i++) {
        broadcast_ip[i] = ip[i] | ~mask[i];
    }
    return broadcast_ip;
}

void init_ethernet();
uint16_t ip_checksum(uint8_t *data, uint32_t length);
uint8_t *get_mac_address();
uint8_t *get_ip_address();

#endif
