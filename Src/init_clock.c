/*
 * Clocks
 * HSI: 8, 16, 32, 64 MHz
 * HSE: 4 - 48 MHz
 * LSE: 32 kHz
 * LSI: 32 kHz
 * CSI: 4 MHz
 * HSI48: 48 MHz
 *
 * memory at 167 MHz
 *
 * preconfigured:
 * HSI: 64MHz
 * HSE: 25MHz
 * LSE: 32.768
 * LSI: 32
 * CSI: 4MHz
 * HSI48: 48MHz
 */

#include <stdint.h>

#define TARGET_SYS_CLK_MHZ 256
#define RCC_REG 0x58024400

void prepare_flash() {
    uint8_t axi_clk = TARGET_SYS_CLK_MHZ / 2;
    if (axi_clk > 135) {
        //update the flash config mate
        //stm32 spec, page 160
        while (1);
    }

    uint32_t flash_base = 0x52002000;
    volatile uint32_t *flash_acr = (uint32_t*)flash_base;
    uint32_t curr_val = *flash_acr;
    curr_val &= ~0b111111;
    uint32_t wrhighfreq = 0b01;
    uint32_t latency = 0b0010;
    curr_val |= (wrhighfreq << 4) | latency;
    *flash_acr = curr_val;

    while (*flash_acr != curr_val);
}

#define SYS_CLK_HSI  = 0b000
#define SYS_CLK_CSI  = 0b001
#define SYS_CLK_HSE  = 0b010
#define SYS_CLK_PLL1 = 0b011
void select_sys_clk(uint8_t clock) {
    volatile uint32_t *rcc_cfgr = (uint32_t *)(RCC_REG + 0x10);
    uint32_t curr = *rcc_cfgr;
    curr &= ~0b111;
    curr |= (clock & 0b111);
    *rcc_cfgr = curr;
}

void init_clock() {
    //optionally change MCO1 and MCO2


    while(1);
}
