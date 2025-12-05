#include <stdint.h>
#include "clock.h"
#include "gpio.h"
#include "register.h"

//0x52004000 - 0x52004FFF FMC control registers
#define FMC_BASE 0x52004000
volatile uint32_t *FMC_BCR1  = (uint32_t *)(FMC_BASE + 0x00);  //bank1 control
volatile uint32_t *FMC_SDCR1 = (uint32_t *)(FMC_BASE + 0x140); //control
volatile uint32_t *FMC_SDCR2 = (uint32_t *)(FMC_BASE + 0x144); //control
volatile uint32_t *FMC_SDTR1 = (uint32_t *)(FMC_BASE + 0x148); //timing
volatile uint32_t *FMC_SDTR2 = (uint32_t *)(FMC_BASE + 0x14C); //timing
volatile uint32_t *FMC_SDCMR = (uint32_t *)(FMC_BASE + 0x150); //command mode
volatile uint32_t *FMC_SDRTR = (uint32_t *)(FMC_BASE + 0x154); //refresh timer
volatile uint32_t *FMC_SDSR  = (uint32_t *)(FMC_BASE + 0x158); //status
volatile uint32_t *MEM_ADDR  = (uint32_t *)(0xD0000000);  //SDRAM base address

//memory clock 100MHz

void set_sdram_pin(uint8_t gpio_class, uint8_t pin_num) {
    set_speed(gpio_class, pin_num, 0b11);
    set_in_pull(gpio_class, pin_num, 0); //no pull
    set_alternate(gpio_class, pin_num, 12);
}

void set_sdram_pins() {
    set_sdram_pin('D', 14);  // D0
    set_sdram_pin('D', 15);  // D1
    set_sdram_pin('D', 0);   // D2
    set_sdram_pin('D', 1);   // D3
    set_sdram_pin('E', 7);   // D4
    set_sdram_pin('E', 8);   // D5
    set_sdram_pin('E', 9);   // D6
    set_sdram_pin('E', 10);  // D7
    set_sdram_pin('E', 11);  // D8
    set_sdram_pin('E', 12);  // D9
    set_sdram_pin('E', 13);  // D10
    set_sdram_pin('E', 14);  // D11
    set_sdram_pin('E', 15);  // D12
    set_sdram_pin('D', 8);   // D13
    set_sdram_pin('D', 9);   // D14
    set_sdram_pin('D', 10);  // D15

    set_sdram_pin('F', 0);   // A0
    set_sdram_pin('F', 1);   // A1
    set_sdram_pin('F', 2);   // A2
    set_sdram_pin('F', 3);   // A3
    set_sdram_pin('F', 4);   // A4
    set_sdram_pin('F', 5);   // A5
    set_sdram_pin('F', 12);  // A6
    set_sdram_pin('F', 13);  // A7
    set_sdram_pin('F', 14);  // A8
    set_sdram_pin('F', 15);  // A9
    set_sdram_pin('G', 0);   // A10
    set_sdram_pin('G', 1);   // A11

    set_sdram_pin('G', 4);   // BA0
    set_sdram_pin('G', 5);   // BA1


    set_sdram_pin('E', 0);   // NBL0
    set_sdram_pin('E', 1);   // NBL1
    set_sdram_pin('H', 7);   // SDCKE1
    set_sdram_pin('G', 8);   // SDCLK
    set_sdram_pin('G', 15);  // SDNCAS
    set_sdram_pin('H', 6);   // SDNE1
    set_sdram_pin('F', 11);  // SDNRAS
    set_sdram_pin('H', 5);   // SDNWE
}

void send_sdram_command(uint32_t command, uint32_t auto_refresh_num, uint32_t mode_reg) {
    set_register(FMC_SDCMR, 0x7FFFFF, 
        (command & 0b111)             | //command
        ((auto_refresh_num - 1) << 5) | //number of auto-refresh cycles
        (1 << 3)                      | //bank 2
        ((mode_reg & 0x3FFF) << 9)      //mode register definition
    );
}

void init_sdram() {
    set_sdram_pins();


    set_register(FMC_SDCR1, (3 << 13) | (1 << 12) | (3 << 10), 
        0b01 << 13 | //RPIPE = 0 clk tick delay    
        0b1  << 12 | //RBURST = no burst mode
        0b10 << 10   //SDCLK = fmc_ker_ck / 2 (100MHz -> 10ns)
    );
    set_register(FMC_SDCR2, 0x7FF, 
        0b0 << 9   | //no write protection
        0b10 << 7  | //CAS 2
        0b1  << 6  | //four internal banks
        0b01 << 4  | //memory bus width 16bits
        0b01 << 2  | //12 row bits
        0b00 << 0    //8 column bits
    );

    set_register(FMC_SDTR1, 1 << 12 | 1 << 20,
        (5 << 12) | //TRC = 6 cycles
        (1 << 20)   //TRP = 2 cycles
    );
    set_register(FMC_SDTR2, 0x0FFFFFFF,
        5 << 24 | //TRCD = 6 cycles
        1 << 16 | //TWR = 2 cycles
        4 << 8  | //TRAS = 5 cycles
        6 << 4  | //TXSR = 7 cycles
        1 << 0    //TMRD = 2 cycles
    );

    set_register(FMC_BCR1, 1U << 31, 1U << 31); //enable fmc

    set_register(FMC_SDRTR, 0x7FFF, 1522 << 1);

    send_sdram_command(0b001, 1, 0); //CLK enable
    wait_ms(1);

    send_sdram_command(0b010, 1, 0); //PALL

    send_sdram_command(0b011, 8, 0); //auto-refresh

    uint32_t mode_reg = (1 << 0) | (2 << 4) | (1 << 9);
    send_sdram_command(0b100, 8, mode_reg);

    //1562
    set_register(FMC_SDRTR, 0x7FFF, 1562 << 1);
}
