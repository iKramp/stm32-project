#include "clock.h"
#include "gpio.h"

#define TARGET_SYS_CLK_MHZ 400

void wait(volatile uint32_t count) {
  while (count--);
}

void reg_read_delay(volatile uint32_t *reg) {
    volatile uint32_t tmp = *reg;
    (void)(tmp);
}

void prepare_flash() {
    uint8_t axi_clk = TARGET_SYS_CLK_MHZ / 2;
    if (axi_clk > 225) {
        // update the flash config mate
        // stm32 spec, page 160
        while (1);
    }

    uint32_t flash_base = 0x52002000;
    volatile uint32_t *flash_acr = (uint32_t *)flash_base;
    uint32_t curr_val = *flash_acr;
    curr_val &= ~0b111111;
    uint32_t wrhighfreq = 0b10;
    uint32_t latency = 0b0100;
    curr_val |= (wrhighfreq << 4) | latency;
    *flash_acr = curr_val;

    while (*flash_acr != curr_val);
}

void select_sys_clk(uint8_t clock) {
    volatile uint32_t *rcc_cfgr = (uint32_t *)(RCC_REG + 0x10);
    set_register(rcc_cfgr, 0b111, clock & 0b111);

    // wait for switch
    while (((*rcc_cfgr >> 3) & 0b111) != (clock & 0b111));
}


void set_domain_config() {
    volatile uint32_t *RCC_D1CFGR = (uint32_t *)(RCC_REG + 0x18);
    volatile uint32_t *RCC_D2CFGR = (uint32_t *)(RCC_REG + 0x1C);
    volatile uint32_t *RCC_D3CFGR = (uint32_t *)(RCC_REG + 0x20);

    set_register(RCC_D1CFGR, 0b111101111111,
        0b0000 << 8 | //D1CPRE = /1
        0b100 << 4 | //D1PPRE  = /2
        0b1000 << 0   //HPRE   = /2
    );

    set_register(RCC_D2CFGR, 0b11101110000, 
        0b100 << 8 | //D2PPRE2 = /2
        0b100 << 4   //D2PPRE1 = /2
    );

    set_register(RCC_D3CFGR, 0b1110000, 
        0b100 << 4 //D3PPRE = /2
    );
}

void configure_pll() {
    volatile uint32_t *RCC_PLLCKSELR = (uint32_t *)(RCC_REG + 0x28);
    volatile uint32_t *RCC_PLLCFGR = (uint32_t *)(RCC_REG + 0x2C);
    volatile uint32_t *RCC_CR = (uint32_t *)(RCC_REG);
    volatile uint32_t *RCC_PLL1DIVR = (uint32_t *)(RCC_REG + 0x30);
    volatile uint32_t *RCC_PLL2DIVR = (uint32_t *)(RCC_REG + 0x38);
    volatile uint32_t *RCC_PLL3DIVR = (uint32_t *)(RCC_REG + 0x40);
    volatile uint32_t *RCC_PLL1FRACR = (uint32_t *)(RCC_REG + 0x34);
    volatile uint32_t *RCC_PLL2FRACR = (uint32_t *)(RCC_REG + 0x3C);
    volatile uint32_t *RCC_PLL3FRACR = (uint32_t *)(RCC_REG + 0x44);

    const uint32_t PLL_ON_MASK = (1 << 28) | (1 << 26) | (1 << 24);
    const uint32_t PLL_RDY_MASK = (1 << 29) | (1 << 27) | (1 << 25);

    // disable outputs
    uint32_t curr = *RCC_PLLCFGR;
    curr &= ~0x01FF0000;
    *RCC_PLLCFGR = curr;

    // wait for PLLs to be disabled
    *RCC_CR &= ~PLL_ON_MASK;
    while (*RCC_CR & PLL_RDY_MASK);

    set_register(RCC_PLL1FRACR, 0xFFF8, 0); // PLL1FRACN = 0
    set_register(RCC_PLL2FRACR, 0xFFF8, 0); // PLL2FRACN = 0
    set_register(RCC_PLL3FRACR, 0xFFF8, 0); // PLL3FRACN = 0

    set_register(RCC_PLLCKSELR, 0x03F3F3F3,
        0b10    | //PLLSRC - HSE
        2 << 4  | //DIVM1 - div by 2
        2 << 12 | //DIVM2 - div by 2
        2 << 20   //DIVM3 - div by 2
    );

    set_register(RCC_PLLCFGR, 0x01FF0FFF,
        0b1100 << 0 | //PLL1 config - in 8-16MHz
        0b1100 << 4 | //PLL2 config - in 8-16MHz
        0b1100 << 8 | //PLL3 config - in 8-16MHz
        0b111 << 16 | //DIV3EN - enable pqr
        0b111 << 19 | //DIV2EN - enable pqr
        0b111 << 22   //DIV1EN - enable pqr
    );

    set_register(RCC_PLL1DIVR, 0x7F7FFFFF, 
        0b111 << 24  | //DIVR1 - divide by 8
        0b1111 << 16 | //DIVQ1 - divide by 16
        0b001 << 9   | //DIVP1 - divide by 2
        64 << 0        //DIVN1 - multiply by 64
    );

    set_register(RCC_PLL2DIVR, 0x7F7FFFFF,
        0b01 << 24 | // DIVR2 - divide by 2
        0b01 << 16 | // DIVQ2 - divide by 2
        0b100 << 9 | // DIVP2 - divide by 5
        15 << 0      // DIVN2 - multiply by 16
    );

    set_register(RCC_PLL3DIVR, 0x7F7FFFFF, 
        0b10 << 24 | // DIVR3 - divide by 3
        0b10 << 16 | // DIVQ3 - divide by 3
        0b01 << 9 |  // DIVP3 - divide by 2
        15 << 0      // DIVN3 - multiply by 16
    );

    *RCC_CR |= PLL_ON_MASK; // enable PLLs
    while ((*RCC_CR & PLL_RDY_MASK) != PLL_RDY_MASK);
}

void set_dom_ker_clk() {
    volatile uint32_t *rcc_d1ccipr = (uint32_t *)(RCC_REG + 0x4C);
    volatile uint32_t *rcc_d2ccipr = (uint32_t *)(RCC_REG + 0x50);
    volatile uint32_t *rcc_d2ccip2r = (uint32_t *)(RCC_REG + 0x54);
    volatile uint32_t *rcc_d3ccipr = (uint32_t *)(RCC_REG + 0x58);

    set_register(rcc_d1ccipr, 0x30010033, 
        0b00 << 28 |  // CKPERSEL = HSI
        0b0 << 17  |  // SDMMCSEL = PLL1Q
        0b00 << 4  |  // QSPISEL = RCC_HCLK3
        0b00 << 0     // FMCSEL = RCC_HCLK3
    );

    set_register(rcc_d2ccipr, 0xB13771C7, 
        0b0 << 31 |   // SWPSEL = PCLK
        0b01 << 28 |  // FDCANSEL = PLL1Q
        0b0 << 25 |   // DFSDM1SEL = rcc_pclk2
        0b00 << 20 |  // SPDIFSEL = PLL1Q
        0b000 << 16 | // SPI45SEL = PCLK
        0b010 << 12 | // SPI123SEL = PLL3P
        0b010 << 6 |  // SAI23SEL = PLL3P
        0b000 << 0    // SAI1SEL = PLL1Q
    );

    set_register(rcc_d2ccip2r, 0x70F0333F, 
        0b000 << 28 | // LPTIM1SEL = rcc_pclk1
        0b01 << 22  | // CECSEL = lsi
        0b11 << 20  | // USBSEL = HSI48
        0b00 << 12  | // I2C123SEL = rcc_pclk1
        0b00 << 8   | // RNGSEL = HSI48
        0b000 << 3  | // USART16SEL = rcc_pclk2
        0b010 << 0    // USART234578SEL = PLL3Q
    );

    set_register(rcc_d3ccipr, 0x77E3FF07, 
        0b000 << 28 | // SPI6SEL = rcc_pclk4
        0b000 << 24 | // SAI4BSEL = PLL1Q
        0b000 << 21 | // SAI4ASEL = PLL1Q
        0b01 << 16  | // ADCSEL = PLL3R
        0b000 << 13 | // LPTIM345SEL = rcc_pclk4
        0b000 << 10 | // LPTIM2SEL = rcc_pclk4
        0b00 << 8   | // I2C4SEL = rcc_pclk4
        0b000 << 0    // LPUART1SEL = rcc_pclk4
    );
}
void enable_clocks() {
    const uint32_t CLK_RDY_MASK = (1 << 17) | (1 << 13) | (1 << 8) | (1 << 2);
    volatile uint32_t *rcc_cr = (uint32_t *)(RCC_REG);
    volatile uint32_t *rcc_csr = (uint32_t *)(RCC_REG + 0x74);

    //TODO: Enable LSI

    set_register(rcc_cr, 0x3F0FF3BF,
        1 << 18 | // HSEBYP
        1 << 16 | // HSEON
        1 << 12 | // HSI48ON
        1 << 7  | // CSION
        1 << 0    // HSION
    );

    set_register(rcc_csr, 1, 1);


    while ((*rcc_cr & CLK_RDY_MASK) != CLK_RDY_MASK);
    while (!(*rcc_csr & 2));
}

void enable_syscfg_clk() {
    volatile uint32_t *RCC_APB4ENR = (uint32_t *)(RCC_REG + 0xF4);
    *RCC_APB4ENR |= 1 << 1; // SYSCFG
}

void change_voltage_scale(uint8_t scale) {
    volatile uint32_t *pwr_d3cr = (uint32_t *)0x58024818;
    volatile uint32_t *pwr_csr1 = (uint32_t *)0x58024804;
    volatile uint32_t *pwr_cr3 = (uint32_t *)0x5802480C;
    volatile uint32_t *syscfg_pwrcr = (uint32_t *)0x5800042c;
    set_register(pwr_cr3, 0b111, 1 << 1);
    while (!(*pwr_csr1 & (1 << 13)));

    set_register(syscfg_pwrcr, 1, 0);
    reg_read_delay(syscfg_pwrcr);
    set_register(pwr_d3cr, 0b11 << 14, (scale & 0b11) << 14);
    reg_read_delay(pwr_d3cr);

    while (!(*pwr_d3cr & (1 << 13)));
}

void enable_peripheral_clocks() {
    volatile uint32_t *RCC_AHB3ENR = (uint32_t *)(RCC_REG + 0xD4);
    *RCC_AHB3ENR |= 1 << 12; // Enable FMC clock
    reg_read_delay(RCC_AHB3ENR);

    volatile uint32_t *RCC_AHB4ENR = (uint32_t *)0x580244E0;
    *RCC_AHB4ENR |= 0b11111111111; // Enable GPIOA to GPIOK clocks
    reg_read_delay(RCC_AHB4ENR);
}


void init_clock() {
    select_sys_clk(SYS_CLK_HSI); // switch to HSI first
    enable_clocks();
    enable_syscfg_clk();
    change_voltage_scale(0b11);
    prepare_flash();
    set_domain_config();
    configure_pll();
    select_sys_clk(SYS_CLK_PLL1);
    set_dom_ker_clk();

    enable_peripheral_clocks();
}
