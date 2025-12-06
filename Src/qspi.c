#include "qspi.h"
#include "clock.h"
#include "gpio.h"
#include "register.h"

#define QSPI_BASE 0x52005000
volatile uint32_t *QSPI_CR = (uint32_t *)(QSPI_BASE + 0x00);
volatile uint32_t *QSPI_DCR = (uint32_t *)(QSPI_BASE + 0x04);
volatile uint32_t *QSPI_SR = (uint32_t *)(QSPI_BASE + 0x8);
volatile uint32_t *QSPI_CCR = (uint32_t *)(QSPI_BASE + 0x14);

//capacity: 128MB
//27 addr bits
//75MHz - 13.33ns

void set_qspi_pin(uint8_t class, uint8_t pin, uint8_t af) {
    set_speed(class, pin, 0b11);
    set_in_pull(class, pin, 0); //no pull
    set_alternate(class, pin, af);
}

void set_qspi_pins() {
    set_qspi_pin('C', 11, 9); //QSPI_BK2_NCS (with SDIO1_D3)
    set_qspi_pin('D', 11, 9); //QSPI_BK1_IO0
    set_qspi_pin('F', 6, 9);  //QSPI_BK1_IO3
    set_qspi_pin('F', 7, 9);  //QSPI_BK1_IO2
    set_qspi_pin('F', 9, 10); //QSPI_BK1_IO1
    set_qspi_pin('F', 10, 9); //QSPI_CLK
    set_qspi_pin('G', 6, 10); //QSPI_BK1_NCS
    set_qspi_pin('G', 9, 9);  //QSPI_BK2_IO2
    set_qspi_pin('G', 14, 9); //QSPI_BK2_IO3
    set_qspi_pin('H', 2, 9);  //QSPI_BK2_IO0 (with MII_CRS)
    set_qspi_pin('H', 3, 9);  //QSPI_BK2_IO1 (with MII_COL)
}

void wait_busy(void) {
    while ((*QSPI_SR) & (1 << 5)); //wait until not busy
}

void init_qspi(void) {
    set_qspi_pins();
    volatile uint32_t *RCC_AHB3RSTR = (uint32_t *)(RCC_REG + 0x7C);
    volatile uint32_t *RCC_AHB3ENR = (uint32_t *)(RCC_REG + 0xD4);

    // enable QSPI clock
    set_register(RCC_AHB3ENR, 1 << 14, 1 << 14);


    // reset QSPI
    set_register(RCC_AHB3RSTR, 1 << 14, 1 << 14);
    set_register(RCC_AHB3RSTR, 1 << 14, 0 << 14);


    wait_busy();
    set_register(QSPI_DCR, 0x1F0701,
        (26 << 16) | //FSIZE: flash size 26 + 1 addr bits
        (3  << 8)  | //CSHT: chip select high time 3+1 cycles
        (0  << 0)    //CKMODE: mode 0
    );
    wait_busy();

    set_register(QSPI_CR, 0xFFDF1FDB,
        (1 << 24) | //PRESCALER: divide by 2 (75MHz)
        (1 << 6)  | //DFM: enable dual flash mode
        (1 << 4)  | //SSHIFT: default setting idk
        (1 << 0)    //EN: disable QSPI
    );
    wait_busy();

    set_register(QSPI_CCR, 0xFF7FFFFF, //enable QUAD IO
        (0 << 31)    | //DDRM: double data rate disable (mem mapped mode)
        (0b00 << 26) | //FMODE: indirect write
        (0b00 << 24) | //DMODE: no data
        (0 << 18)    | //DCYC: 0 dummy cycles
        (0 << 14)    | //ABMODE: no alternate bytes
        (0b00 << 10) | //ADMODE: no address
        (0b01 << 8)  | //IMODE: instruction on 1 line
        (0x35 << 0)    //INSTRUCTION: ENTER QUAD IO MODE
    );
    wait_busy();

    set_register(QSPI_CCR, 0xFF7FFFFF, //enable 4byte address mode
        (0 << 31)    | //DDRM: double data rate disable (mem mapped mode)
        (0b00 << 26) | //FMODE: indirect write
        (0b00 << 24) | //DMODE: no data
        (0 << 18)    | //DCYC: 0 dummy cycles
        (0 << 14)    | //ABMODE: no alternate bytes
        (0b00 << 10) | //ADMODE: no address
        (0b11 << 8)  | //IMODE: instruction on 4 lines
        (0xB7 << 0)    //INSTRUCTION: ENTER 4BYTE ADDRESS MODE
    );
    wait_busy();

    set_register(QSPI_CCR, 0xFF7FFFFF, 
        (0 << 31)    | //DDRM: double data rate disable (mem mapped mode)
        (0 << 30)    | //DHHC: idk
        (0b11 << 26) | //FMODE: memory mapped
        (0b11 << 24) | //DMODE: data on 4 lines
        (10 << 18)   | //DCYC: 10 dummy cycles
        (0 << 16)    | //ABSIZE: no alternate bytes
        (0 << 14)    | //ABMODE: no alternate bytes
        (0b11 << 12) | //ADSIZE: 32 bit address
        (0b11 << 10) | //ADMODE: address on 4 lines
        (0b11 << 8)  | //IMODE: instruction on 4 lines
        (0xEC << 0)    //INSTRUCTION: 4byte Quad I/O Fast Read
    );
    wait_busy();
}
