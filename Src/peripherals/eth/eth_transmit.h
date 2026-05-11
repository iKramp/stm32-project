#ifndef ETH_TRANSMIT_H
#define ETH_TRANSMIT_H

#include <stdint.h>

void init_eth_tx();
void set_tx_state(uint8_t enabled);
uint8_t transmit_data(uint8_t *data, uint32_t length);
uint8_t transmit_data_udp(uint8_t *payload, uint32_t payload_len, uint8_t *dest_ip, uint8_t *dest_mac, uint16_t dest_port);
uint8_t get_tx_state();

#endif // ETH_TRANSMIT_H
