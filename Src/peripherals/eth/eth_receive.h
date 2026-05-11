#ifndef ETH_RECEIVE_H
#define ETH_RECEIVE_H
#include <stdint.h>

void init_eth_rx();
void set_rx_state(uint8_t enabled);
uint8_t get_rx_state();
uint32_t receive_packet_data(uint8_t *buffer);
void debug_receive_queue();

#endif // ETH_RECEIVE_H
