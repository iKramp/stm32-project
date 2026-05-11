#ifndef SRAM_H
#define SRAM_H

#include <stdint.h>

uintptr_t eth_rx_queue_addr();
uintptr_t eth_tx_queue_addr();
uintptr_t eth_rx_data_start();
uintptr_t eth_tx_data_start();

#endif // SRAM_H
