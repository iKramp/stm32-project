/*
 * preconfigured:
 * HSI: 64MHz
 * HSE: 25MHz
 * LSE: 32.768
 * LSI: 32
 * CSI: 4MHz
 * HSI48: 48MHz
 *
 * 
 */

#include <stdint.h>

#define TARGET_SYS_CLK_MHZ 400 //MHZ
//AXI-CLK = SYS_CLK / 2
#define RCC_REG 0x58024400

void prepare_flash() {
    uint8_t axi_clk = TARGET_SYS_CLK_MHZ / 2;
    if (axi_clk > 225) {
        //update the flash config mate
        //stm32 spec, page 160
        while (1);
    }

    uint32_t flash_base = 0x52002000;
    volatile uint32_t *flash_acr = (uint32_t*)flash_base;
    uint32_t curr_val = *flash_acr;
    curr_val &= ~0b111111;
    uint32_t wrhighfreq = 0b10;
    uint32_t latency = 0b0100;
    curr_val |= (wrhighfreq << 4) | latency;
    *flash_acr = curr_val;

    while (*flash_acr != curr_val);
}

#define SYS_CLK_HSI  0b000
#define SYS_CLK_CSI  0b001
#define SYS_CLK_HSE  0b010
#define SYS_CLK_PLL1 0b011
void select_sys_clk(uint8_t clock) {
    volatile uint32_t *rcc_cfgr = (uint32_t *)(RCC_REG + 0x10);
    uint32_t curr = *rcc_cfgr;
    curr &= ~0b111;
    curr |= (clock & 0b111);
    *rcc_cfgr = curr;
    //wait for switch
    while ((((*rcc_cfgr) & (0b111 << 3)) >> 3) != (clock & 0b111));
}

void set_domain_config() {
    volatile uint32_t *rcc_dom1_cfgr = (uint32_t *)(RCC_REG + 0x18);
    volatile uint32_t *rcc_dom2_cfgr = (uint32_t *)(RCC_REG + 0x1C);
    volatile uint32_t *rcc_dom3_cfgr = (uint32_t *)(RCC_REG + 0x20);

    uint32_t curr = *rcc_dom1_cfgr;
    curr &= ~0b111101111111; //clear
    curr |= 0b0000 << 8; //D1CPRE = /1
    curr |= 0b0100 << 4; //D1PPRE = /2
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
    volatile uint32_t *rcc_pll1divr = (uint32_t *)(RCC_REG + 0x30);
    volatile uint32_t *rcc_pll2divr = (uint32_t *)(RCC_REG + 0x38);
    volatile uint32_t *rcc_pll3divr = (uint32_t *)(RCC_REG + 0x40);
    volatile uint32_t *rcc_pll1fracr = (uint32_t *)(RCC_REG + 0x34);
    volatile uint32_t *rcc_pll2fracr = (uint32_t *)(RCC_REG + 0x3C);
    volatile uint32_t *rcc_pll3fracr = (uint32_t *)(RCC_REG + 0x44);

    const uint32_t PLL_ON_MASK = (1 << 28) | (1 << 26) | (1 << 24);
    const uint32_t PLL_RDY_MASK = (1 << 29) | (1 << 27) | (1 << 25);

    //disable outputs
    uint32_t curr = *rcc_pllcfgr;
    curr &= ~0x01FF0FFF;
    *rcc_pllcfgr = curr;

    //wait for PLLs to be disabled
    *rcc_cr &= ~PLL_ON_MASK;
    while (*rcc_cr & PLL_RDY_MASK);

    //pre-init fraction
    curr = *rcc_pll1fracr;
    curr &= ~0xFFF8; //clear
    *rcc_pll1fracr = curr; //PLL1FRACN = 0
    curr = *rcc_pll2fracr;
    curr &= ~0xFFF8; //clear
    *rcc_pll2fracr = curr; //PLL2FRACN = 0
    curr = *rcc_pll3fracr;
    curr &= ~0xFFF8; //clear
    *rcc_pll3fracr = curr; //PLL3FRACN = 0

    curr = *rcc_pllckselr;
    curr &= ~0x03F3F3F3;
    curr |= 0b10; //PLLSRC - HSE
    curr |= 2 << 4; //DIVM1 - div by 2
    curr |= 2 << 12; //DIVM2 - div by 2
    curr |= 2 << 20; //DIVM3 - div by 2
    *rcc_pllckselr = curr;

    curr = *rcc_pllcfgr;
    curr &= ~0x01FF0FFF;

    curr |= 0b1100 << 0; //PLL1 config - in 8-16MHz
    curr |= 0b1100 << 4; //PLL2 config - in 8-16MHz
    curr |= 0b1100 << 8; //PLL3 config - in 8-16MHz

    curr |= 0b111 << 22; //DIV3EN - enable pqr
    curr |= 0b111 << 19; //DIV2EN - enable pqr
    curr |= 0b111 << 16; //DIV1EN - enable pqr
    *rcc_pllcfgr = curr;

    curr = *rcc_pll1divr;
    curr &= ~0x7F7FFFFF; //clear
    curr |= 0b111 << 24; //DIVR1 - divide by 8
    curr |= 0b1111 << 16; //DIVQ1 - divide by 16
    curr |= 0b001 << 9; //DIVP1 - divide by 2
    curr |= 64 << 0; //DIVN1 - multiply by 64
    *rcc_pll1divr = curr;

    curr = *rcc_pll2divr;
    curr &= ~0x7F7FFFFF; //clear
    curr |= 0b01 << 24; //DIVR2 - divide by 2
    curr |= 0b01 << 16; //DIVQ2 - divide by 2
    curr |= 0b100 << 9; //DIVP2 - divide by 5
    curr |= 15 << 0; //DIVN2 - multiply by 16
    *rcc_pll2divr = curr;

    curr = *rcc_pll3divr;
    curr &= ~0x7F7FFFFF; //clear
    curr |= 0b10 << 24; //DIVR3 - divide by 3
    curr |= 0b10 << 16; //DIVQ3 - divide by 3
    curr |= 0b01 << 9; //DIVP3 - divide by 2
    curr |= 15 << 0; //DIVN3 - multiply by 16
    *rcc_pll3divr = curr;

    *rcc_cr |= PLL_ON_MASK; //enable PLLs
    while ((*rcc_cr & PLL_RDY_MASK) != PLL_RDY_MASK);
}

void set_dom_ker_clk() {
    volatile uint32_t *rcc_d1ccipr = (uint32_t *)(RCC_REG + 0x4C);
    volatile uint32_t *rcc_d2ccipr = (uint32_t *)(RCC_REG + 0x50);
    volatile uint32_t *rcc_d2ccip2r = (uint32_t *)(RCC_REG + 0x54);
    volatile uint32_t *rcc_d3ccipr = (uint32_t *)(RCC_REG + 0x58);

    uint32_t curr = *rcc_d1ccipr;
    curr &= ~0x30010033; //clear
    curr |= 0b00 << 28; //CKPERSEL = HSI
    curr |= 0b0 << 17; //SDMMCSEL = PLL1Q
    curr |= 0b00 << 4; //QSPISEL = RCC_HCLK3
    curr |= 0b00 << 0; //FMCSEL = RCC_HCLK3
    *rcc_d1ccipr = curr;

    curr = *rcc_d2ccipr;
    curr &= ~0xB13771C7; //clear
    curr |= 0b0 << 31; //SWPSEL = PCLK
    curr |= 0b01 << 28; //FDCANSEL = PLL1Q
    curr |= 0b0 << 25; //DFSDM1SEL = rcc_pclk2
    curr |= 0b00 << 20; //SPDIFSEL = PLL1Q
    curr |= 0b000 << 16; //SPI45SEL = PCLK
    curr |= 0b010 << 12; //SPI123SEL = PLL3P
    curr |= 0b010 << 6; //SAI23SEL = PLL3P
    curr |= 0b000 << 0; //SAI1SEL = PLL1Q
    *rcc_d2ccipr = curr;

    curr = *rcc_d2ccip2r;
    curr &= ~0x70F0333F; //clear
    curr |= 0b000 << 28; //LPTIM1SEL = rcc_pclk1
    curr |= 0b01 << 22; //CECSEL = lsi
    curr |= 0b11 << 20; //USBSEL = HSI48
    curr |= 0b00 << 12; //I2C123SEL = rcc_pclk1
    curr |= 0b00 << 8; //RNGSEL = HSI48
    curr |= 0b000 << 3; //USART16SEL = rcc_pclk2
    curr |= 0b010 << 0; //USART234578SEL = PLL3Q
    *rcc_d2ccip2r = curr;

    curr = *rcc_d3ccipr;
    curr &= ~0x77E3FF07;
    curr |= 0b000 << 28; //SPI6SEL = rcc_pclk4
    curr |= 0b000 << 24; //SAI4BSEL = PLL1Q
    curr |= 0b000 << 21; //SAI4ASEL = PLL1Q
    curr |= 0b01 << 16; //ADCSEL = PLL3R
    curr |= 0b000 << 13; //LPTIM345SEL = rcc_pclk4
    curr |= 0b000 << 10; //LPTIM2SEL = rcc_pclk4
    curr |= 0b00 << 8; //I2C4SEL = rcc_pclk4
    curr |= 0b000 << 0; //LPUART1SEL = rcc_pclk4
    *rcc_d3ccipr = curr;
}

// void change_vos() {
//     volatile uint32_t *pwr_apb4enr = (uint32_t *)(RCC_REG + 0xF4);
//     *pwr_apb4enr |= 1 << 1; //syscfg
//
//
//     volatile uint32_t *pwr_d3cr = (uint32_t *)(0x58024800 + 0x18);
//     uint32_t curr = *pwr_d3cr;
//     curr &= ~(0b11 << 14); //clear
//     curr |= 0b11 << 14; //VOS = Scale 1
//     *pwr_d3cr = curr;
//     while (!(*pwr_d3cr & (1 << 13)));
// }

void enable_clocks() {
    const uint32_t CLK_RDY_MASK = (1 << 17) | (1 << 13) | (1 << 8) | (1 << 2);

    volatile uint32_t *rcc_cr = (uint32_t *)(RCC_REG);
    uint32_t curr = *rcc_cr;
    curr &= ~(1 << 18); //no HSE bypass
    curr |= 1 << 16; //HSEON
    curr |= 1 << 12; //HSI48ON
    curr |= 1 << 7; //CSION
    curr |= 1 << 0; //HSION
    *rcc_cr = curr;

    while ((*rcc_cr & CLK_RDY_MASK) != CLK_RDY_MASK);
}

void enable_peripheral_clocks() {
    volatile uint32_t *RCC_AHB3ENR = (uint32_t *)(RCC_REG + 0xD4);
    *RCC_AHB3ENR |= 1 << 12; // Enable FMC clock

    volatile uint32_t *RCC_AHB4ENR = (uint32_t *)0x580244E0;
    *RCC_AHB4ENR |= 0b1111111111; // Enable GPIOA to GPIOK clocks

    volatile uint32_t *RCC_APB4ENR = (uint32_t *)(RCC_REG + 0xF4);
    *RCC_APB4ENR |= 1 << 0; //SYSCFG
    *RCC_APB4ENR |= 1 << 2; //PWR
}

void init_clock() {
    select_sys_clk(SYS_CLK_HSI);  // switch to HSI first
    // change_vos();
    enable_clocks();
    prepare_flash();
    set_domain_config();
    configure_pll();
    select_sys_clk(SYS_CLK_PLL1);
    set_dom_ker_clk();
    enable_peripheral_clocks();
}
