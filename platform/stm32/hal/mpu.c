#include "mpu.h"
#include "common.h"
#include "../rendering/framebuffer.h"
#include "../peripherals/sram.h"
#include <stdint.h>
#include <stdio.h>

volatile uint32_t *MPU_TYPE = (uint32_t *)0xE000ED90;
volatile uint32_t *MPU_CTRL = (uint32_t *)0xE000ED94;
volatile uint32_t *MPU_RNR = (uint32_t *)0xE000ED98;
volatile uint32_t *MPU_RBAR = (uint32_t *)0xE000ED9C;
volatile uint32_t *MPU_RASR = (uint32_t *)0xE000EDA0;

void set_mpu_region(
        uint8_t region_number, 
        uint32_t base_address, 
        uint32_t size,
        uint8_t subregion_disable_mask,
        uint16_t attributes
) {
    uint32_t size_power = 0;
    uint32_t size_temp = size;
    while (size_temp > 1) {
        size_temp >>= 1;
        size_power++;
    }
    *MPU_RNR = region_number;
    *MPU_RBAR = base_address;
    *MPU_RASR =
        attributes << 16 | //attributes in upper half
        (subregion_disable_mask << 8) |
        (size_power - 1) |
        1; //enable region

}

//attrs:
//[15, 13] - reserved
//12 - execute disable
//11 - reserved
//[10, 8] - access permissions
// 0b000 - no access
// 0b001 - privileged read/write, unprivileged no access
// 0b010 - privileged read/write, unprivileged read-only
// 0b011 - privileged read/write, unprivileged read/write
// 0b100 - reserved
// 0b101 - privileged read-only, unprivileged no access
// 0b110 - privileged read-only, unprivileged read-only
// 0b111 - privileged read-only, unprivileged read-only
// [7, 6] - reserved
// [5, 3] - TEX
// 2 - shareable
// 1 - C
// 0 - B
// consult the ARMv7-M Architecture Reference Manual, page 641

void init_mpu() {
    uint32_t mpu_type = *MPU_TYPE;
    uint8_t dregions = (mpu_type >> 8) & 0xFF;

    uint32_t mpu_ctrl = *MPU_CTRL;
    uint32_t mpu_rnr = *MPU_RNR;
    uint32_t mpu_rbar = *MPU_RBAR;
    uint32_t mpu_rasr = *MPU_RASR;

    printf("MPU Type: 0x%08X\n", mpu_type);
    printf("MPU Control: 0x%08X\n", mpu_ctrl);

    uint16_t mmio_attrs = 
        (1 << 12) | //execute disable
        (0b011 << 8) | //privileged read/write, unprivileged read/write
        (0b000 << 3) | //TEX = 0 (Strongly ordered)
        (0 << 1) | //not cacheable
        (0 << 0); //not bufferable

    uint16_t fb_attrs = 
        (1 << 12) | //executable
        (0b011 << 8) | //privileged read/write, unprivileged read/write
        (0b000 << 3) | //TEX = 0 (Strongly ordered)
        (1 << 2) | //shareable
        (1 << 1) | //cacheable
        (0 << 0); //not bufferable

    uint16_t eth_buf_attrs = 
        (1 << 12) | //execute disable
        (0b011 << 8) | //privileged read/write, unprivileged read/write
        (0b000 << 3) | //TEX = 0 (Strongly ordered)
        (1 << 2) | //shareable
        (0 << 1) | //not cacheable
        (0 << 0); //not bufferable

    //registers from 0x40000000 to 0x580273FF, round to 0x40000000 to 0x6000000

    set_mpu_region(0, 0x40000000, 0x2000000, 0, mmio_attrs);

    struct FrameBuffer *fb = get_fb();
    uint32_t fb_addr = (uint32_t)(uintptr_t)fb->buffer;
    uint32_t fb_size = fb->width * fb->height * fb->bpp;
    //round to next pow of 2
    uint32_t fb_size_pow2 = 1;
    while (fb_size_pow2 < fb_size) {
        fb_size_pow2 <<= 1;
    }
    printf("Framebuffer address: 0x%08X, size: %X bytes, rounded size: %X bytes\n", fb_addr, fb_size, fb_size_pow2);
    set_mpu_region(1, fb_addr, fb_size_pow2, 0, fb_attrs); //should set 1MB

    uintptr_t eth_buf_begin = eth_rx_data_start();
    uintptr_t eth_buf_end = get_sram_end();

    uint32_t eth_buf_size = eth_buf_end - eth_buf_begin;
    //round to next pow of 2
    uint32_t eth_buf_size_pow2 = 1;
    while (eth_buf_size_pow2 < eth_buf_size) {
        eth_buf_size_pow2 <<= 1;
    }
    printf("Ethernet buffers address: 0x%08X, size: %X bytes, rounded size: %X bytes\n", (uint32_t)eth_buf_begin, eth_buf_size, eth_buf_size_pow2);
    set_mpu_region(2, eth_buf_begin, eth_buf_size_pow2, 0, eth_buf_attrs);

    //lastly, enable MPU
    *MPU_CTRL = mpu_ctrl | 0b111;

    panic("MPU initialized");
}
