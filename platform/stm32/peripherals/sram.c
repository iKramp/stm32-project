#include "sram.h"
#include "../hal/common.h"

extern char _sram_d2;
uintptr_t sram_d2_addr = 0;

uintptr_t eth_rx_data_start() {
    if (sram_d2_addr == 0) {
        sram_d2_addr = (uintptr_t)&_sram_d2;
    }

    return align_addr(sram_d2_addr, 1024);
}

uintptr_t eth_tx_data_start() {
    return eth_rx_data_start() + 128 * 1024; //each data block is 1024B, 128 descriptors
}

uintptr_t eth_rx_queue_addr() {
    return eth_tx_data_start() + 128 * 1024; //each data block is 1024B, 128 descriptors
}

uintptr_t eth_tx_queue_addr() {
    return eth_rx_queue_addr() + 128 * 16; //128 descriptors of size 16B
}

uintptr_t get_sram_end() {
    return eth_tx_queue_addr() + 128 * 16;
}
