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

void set_domain_config() {
    volatile uint32_t *rcc_dom1_cfgr = (uint32_t *)(RCC_REG + 0x18);
    volatile uint32_t *rcc_dom2_cfgr = (uint32_t *)(RCC_REG + 0x1C);
    volatile uint32_t *rcc_dom3_cfgr = (uint32_t *)(RCC_REG + 0x20);

    uint32_t curr = *rcc_dom1_cfgr;
    curr &= ~0b111101111111; //clear
    curr |= 0b0000 << 8; //D1CPRE = /1
    curr |= 0b100 << 4; //D1PPRE = /2
    curr |= 0b1000; //HPRE = /2
    *rcc_dom1_cfgr = curr;

    curr = *rcc_dom2_cfgr;
    curr &= ~0b11101110000; //clear
    curr |= 0b100 << 8; //D2PPRE2 = /2
    curr |= 0b100 << 4; //D2PPRE1 = /2
    *rcc_dom2_cfgr = curr;

    curr = *rcc_dom3_cfgr;
    curr &= ~0b1110000; //clear
    curr |= 0b100 << 4; //D3PPRE = /2
    *rcc_dom3_cfgr = curr;
}

void configure_pll() {
    volatile uint32_t *rcc_pllckselr = (uint32_t *)(RCC_REG + 0x28);
    volatile uint32_t *rcc_pllcfgr = (uint32_t *)(RCC_REG + 0x2C);
    volatile uint32_t *rcc_cr = (uint32_t *)(RCC_REG);

    uint32_t curr = *rcc_pllckselr;
    curr &= ~0x03F3F3F3;
    curr |= 0b10; //PLLSRC
    curr |= 0b10 << 4; //DIVM1
    curr |= 0b10 << 12; //DIVM2
    curr |= 0b100000 << 20; //DIVM3
    *rcc_pllckselr = curr;

    //wait for PLLs to be disabled
    *rcc_cr &= ~((1 << 28) | (1 << 26) | (1 << 24));
    while (*rcc_cr & ((1 << 29) | (1 << 27) | (1 << 25)));

    curr = *rcc_pllcfgr;
    curr &= ~0x01FF0FFF;

}

void init_clock() {
    //optionally change MCO1 and MCO2
    prepare_flash();

    while(1);
}
