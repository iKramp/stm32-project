#include <stdint.h>
#include "gpio.h"

//0x52004000 - 0x52004FFF FMC control registers
#define FMC_BASE 0x52004000
volatile uint32_t *FMC_SDCR1 = (uint32_t *)(FMC_BASE + 0x140); //control
volatile uint32_t *FMC_SDCR2 = (uint32_t *)(FMC_BASE + 0x144); //control
volatile uint32_t *FMC_SDTR1 = (uint32_t *)(FMC_BASE + 0x148); //timing
volatile uint32_t *FMC_SDTR2 = (uint32_t *)(FMC_BASE + 0x14C); //timing
volatile uint32_t *FMC_SDCMR = (uint32_t *)(FMC_BASE + 0x150); //command mode
volatile uint32_t *FMC_SDRTR = (uint32_t *)(FMC_BASE + 0x154); //refresh timer
volatile uint32_t *FMC_SDSR  = (uint32_t *)(FMC_BASE + 0x158); //status
volatile uint32_t *MEM_ADDR  = (uint32_t *)(0xD0000000);  //SDRAM base address

//memory clock 100MHz

void set_pin(uint8_t gpio_class, uint8_t pin_num) {
    set_speed(gpio_class, pin_num, 0b11);
    set_alternate(gpio_class, pin_num, 12);
}

void set_sdram_pins() {
    // SDRAM data lines
    set_pin('D', 0);  // D0
    set_pin('D', 1);  // D1
    set_pin('D', 8);  // D2
    set_pin('D', 9);  // D3
    set_pin('D', 10); // D4
    set_pin('D', 14); // D5
    set_pin('D', 15); // D6
    set_pin('E', 0);  // D7
    set_pin('E', 1);  // D8
    set_pin('E', 7);  // D9
    set_pin('E', 8);  // D10
    set_pin('E', 9);  // D11
    set_pin('E', 10); // D12
    set_pin('E', 11); // D13
    set_pin('E', 12); // D14
    set_pin('E', 13); // D15
    set_pin('E', 14); // D16
    set_pin('E', 15); // D17
    set_pin('F', 0);  // D18
    set_pin('F', 1);  // D19
    set_pin('F', 2);  // D20
    set_pin('F', 3);  // D21
    set_pin('F', 4);  // D22
    set_pin('F', 5);  // D23
    set_pin('F', 11); // D24
    set_pin('F', 12); // D25
    set_pin('F', 13); // D26
    set_pin('F', 14); // D27
    set_pin('F', 15); // D28
    set_pin('G', 0);  // D29
    set_pin('G', 1);  // D30
    set_pin('G', 2);  // D31
    
    // Address lines
    set_pin('G', 3);  // A0
    set_pin('G', 4);  // A1
    set_pin('G', 5);  // A2
    set_pin('G', 8);  // A3
    set_pin('G', 15); // A4
    set_pin('H', 5);  // A5
    set_pin('H', 6);  // A6
    set_pin('H', 7);  // A7
    
    // Control signals
    set_pin('C', 0);  // SDNWE
    set_pin('C', 2);  // SDNE0
    set_pin('C', 3);  // SDCK
    set_pin('D', 3);  // SDCAS
    set_pin('D', 4);  // SDRAS
    set_pin('G', 9);  // SDCKE0
    set_pin('G', 10); // SDCLK
    set_pin('G', 12); // BA0
    set_pin('G', 13); // BA1
    set_pin('G', 14); // NBL0
    set_pin('H', 3);  // NBL1
}

void init_sdram() {
    set_sdram_pins();

    uint32_t curr = *FMC_SDCR1;
    curr &= ~0x7FFF; //clear
    curr |=
        (0b01 << 13) | //1 clk tick delay
        (0b0  << 12) | //no burst mode
        (0b10 << 10) | //SDCLK = fmc_ker_ck / 2 (100MHz -> 10ns)
        (0b0  << 9)  | //no write protection
        (0b10 << 7)  | //CAS 2
        (0b1  << 6)  | //four internal banks
        (0b10 << 4)  | //memory bus width 32bits
        (0b01 << 2)  | //12 row bits
        (0b00 << 0);   //8 column bits
    *FMC_SDCR1 = curr;
    *FMC_SDCR2 = curr;

    curr = *FMC_SDTR1;
    curr &= ~0x0FFFFFFF; //clear
    curr |=
        (0b0001 << 24) | //TRCD = 2 cycles
        (0b0001 << 20) | //TRP = 2 cycles
        (0b0001 << 16) | //TWR = 2 cycles
        (0b0110 << 12) | //TRC = 7 cycles
        (0b0100 << 8)  | //TRAS = 5 cycles
        (0b0110 << 4)  | //TXSR = 7 cycles
        (0b0001 << 0);   //TMRD = 2 cycles
    *FMC_SDTR1 = curr;
    *FMC_SDTR2 = curr;

    curr = *FMC_SDCMR;
    curr &= ~0x7FFFFF; //clear
    curr |= 0b001; //clock configuration enable
    curr |= 1 << 3; //bank 2
    *FMC_SDCMR = curr;
    *MEM_ADDR = 1; //dummy write
    while (*FMC_SDSR & 0x0000001F); //wait until mode bit cleared
    for (volatile int i = 0; i < 1000; i++);

    //power up delay
    //40000 clock cycles
    //assume decrement, jump
    //20000 cycles
    for (volatile int i = 0; i < 80000; i++);

    curr = *FMC_SDCMR;
    curr &= ~0x7FFFFF; //clear
    curr |= 0b010; //precharge all command
    curr |= 1 << 3; //bank 2
    *FMC_SDCMR = curr;
    *MEM_ADDR = 1; //dummy write
    while (*FMC_SDSR & 0x0000001F); //wait until mode bit cleared
    for (volatile int i = 0; i < 1000; i++);

    curr = *FMC_SDCMR;
    curr &= ~0x7FFFFF; //clear
    curr |= 0b011; //auto-refresh command
    curr |= 1 << 5; //number of auto-refresh cycles = 2
    curr |= 1 << 3; //bank 2
    *FMC_SDCMR = curr;
    *MEM_ADDR = 1; //dummy write
    while (*FMC_SDSR & 0x0000001F); //wait until mode bit cleared
    for (volatile int i = 0; i < 1000; i++);

    curr = *FMC_SDCMR;
    curr &= ~0x7FFFFF; //clear
    curr |= 0b100; //load mode register

    uint32_t mode_reg = 
        (0b010 << 0) | //burst length = 4
        (0b0   << 3) | //burst type = sequential
        (0b010 << 4) | //CAS latency = 2
        (0b00  << 7) | //operating mode = standard
        (0b0   << 9);  //write burst mode = programmed burst length
    curr |= (mode_reg << 9); //mode register
    curr |= 1 << 3; //bank 2
    *FMC_SDCMR = curr;
    *MEM_ADDR = 1; //dummy write
    while (*FMC_SDSR & 0x0000001F); //wait until mode bit cleared
    for (volatile int i = 0; i < 1000; i++);

    curr = *FMC_SDRTR;
    curr &= ~0x7FFF; //clear
    curr |= 1522 << 1;
    *FMC_SDRTR = curr;
}
