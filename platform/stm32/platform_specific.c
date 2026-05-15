#include "../../include/platform_specific.h"
#include "../../core/scene_data.h"
#include "hal/fpu.h"
#include "hal/mpu.h"
#include "peripherals/clock.h"
#include "peripherals/fmc.h"
#include "peripherals/qspi.h"
#include "rendering/framebuffer.h"
#include "peripherals/ltdc.h"
#include "peripherals/eth/ethernet.h"
#include "hal/common.h"

#define QSPI_ADDR 0x90000000

uint8_t *round_to_boundary(uint8_t *ptr, uint8_t boundary_size) {
    uintptr_t addr = (uintptr_t)ptr;
    if (addr % boundary_size == 0) {
        return ptr;
    }
    uintptr_t rounded_addr = (addr + boundary_size - 1) & ~(boundary_size - 1);
    return (uint8_t *)rounded_addr;
}

uint32_t _get_scene_data_size() {
    uint8_t *data_ptr = (uint8_t *)QSPI_ADDR;
    uint8_t *original_ptr = data_ptr;
    uint32_t total_size = 0;
    for (int i = 0; i < 7; i++) {
        uint32_t buffer_size = *((uint32_t *)data_ptr);
        data_ptr = round_to_boundary(data_ptr + 4, 16);
        data_ptr = round_to_boundary(data_ptr + buffer_size, 4);

    }
    total_size = data_ptr - original_ptr;
    return total_size;
}

void platform_init(uint8_t server) {
    init_fpu();

    init_clock();

    init_sdram();
    init_qspi();

    clear_framebuffer(0xFF000000);
    init_display();

    init_mpu();
    enable_caches();

    init_ethernet();

    set_scene_data((uint8_t *)QSPI_ADDR, _get_scene_data_size());
}

uint8_t *get_data_buffer() {
    return (uint8_t *)SCENE_DATA_ADDR;
}
