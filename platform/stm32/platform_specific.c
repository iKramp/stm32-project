#include "../../include/platform_specific.h"
#include "../../core/scene_data.h"
#include "hal/mpu.h"
#include "peripherals/clock.h"
#include "peripherals/fmc.h"
#include "peripherals/qspi.h"
#include "rendering/framebuffer.h"
#include "peripherals/ltdc.h"
#include "peripherals/eth/ethernet.h"
#include "hal/common.h"

#define QSPI_ADDR 0x90000000

uint32_t _get_scene_data_size() {
    uint8_t *data_ptr = (uint8_t *)QSPI_ADDR;
    uint32_t total_size = 0;
    for (int i = 0; i < 7; i++) {
        uint32_t buffer_size = *((uint32_t *)data_ptr);
        total_size += 4 + buffer_size; //size field + buffer data
        data_ptr += 4 + buffer_size;
    }
    return total_size;
}

void platform_init(uint8_t server) {

    init_clock();

    init_sdram();
    init_qspi();

    clear_framebuffer(0xFF000000);
    init_display();

    // init_mpu();
    // enable_caches();
    //
    init_ethernet();

    set_scene_data((uint8_t *)QSPI_ADDR, _get_scene_data_size());
}

uint8_t *get_data_buffer() {
    return (uint8_t *)SCENE_DATA_ADDR;
}
