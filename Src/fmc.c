#include <stdint.h>

//0x52004000 - 0x52004FFF FMC control registers
const uint32_t FMC_BASE = 0x52004000;
volatile uint32_t *FMC_SDCR1 = (uint32_t *)(FMC_BASE + 0x140); //control
volatile uint32_t *FMC_SDTR1 = (uint32_t *)(FMC_BASE + 0x148); //timing
volatile uint32_t *FMC_SDCMR = (uint32_t *)(FMC_BASE + 0x150); //command mode
volatile uint32_t *FMC_SDRTR = (uint32_t *)(FMC_BASE + 0x154); //refresh timer
volatile uint32_t *FMC_SDSR  = (uint32_t *)(FMC_BASE + 0x158); //status

void init_sdram() {
    *FMC_SDCMR |= 1 << 4; //control bank 1

    uint32_t fmc_sdcr1 = 0x0 |
        (0b01 << 13) | //delay
                       //no burst
        (0b10 << 10) | //clock
                       //no write protection
        (0b01 << 07) | //default CAS latency
        (0b01 << 06) | //4 internal banks
        (0b10 << 04) | //32bit data width
        (0b01 << 02) | //12 bit rows
        (0b00 << 00);  //8 bit columns

    *FMC_SDCR1 = fmc_sdcr1;
    
    while (1) ; //not finished
}
