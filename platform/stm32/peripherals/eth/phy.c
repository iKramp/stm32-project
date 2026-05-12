#include "phy.h"
#include "ethernet.h"
#include <stdio.h>

static volatile uint32_t *ETH_MACMDIOAR = (uint32_t *)(ETHERNET_BASE + 0x0200);
static volatile uint32_t *ETH_MACMDIODR = (uint32_t *)(ETHERNET_BASE + 0x0204);

static uint8_t PHY_ADDR = 0x0;

uint16_t mdio_read(uint8_t reg_addr) {
    while (*ETH_MACMDIOAR & 1) {} //wait until not busy

    set_register(ETH_MACMDIOAR, 0xFFF701F, 
        0 << 27 | //PSE
        0 << 26 | //BTB
        PHY_ADDR << 21 | //PA
        reg_addr << 16 | //RDA
        3 << 12 | //NTC
        0 << 04 | //SKAP
        3 << 02 | //GOC
        0 << 01 | //C45E
        1 << 00   //MB
    );

    //wait for operation to complete
    while (*ETH_MACMDIOAR & 1) {}

    //read data from MDIOAR
    return *ETH_MACMDIODR & 0xFFFF;
}

void mdio_write(uint8_t reg_addr, uint16_t data) {
    while (*ETH_MACMDIOAR & 1) {} //wait until not busy

    //write data to MDIODR
    *ETH_MACMDIODR = data;

    set_register(ETH_MACMDIOAR, 0xFFF701F, 
        0 << 27 | //PSE
        0 << 26 | //BTB
        PHY_ADDR << 21 | //PA
        reg_addr << 16 | //RDA
        3 << 12 | //NTC
        0 << 04 | //SKAP
        1 << 02 | //GOC
        0 << 01 | //C45E
        1 << 00   //MB
    );

    while (*ETH_MACMDIOAR & 1) {} //wait until not busy
}

void init_phy() {
    for (int phy_addr = 0; phy_addr < 32; phy_addr++) {
        PHY_ADDR = phy_addr;
        uint16_t org_id = mdio_read(2);
        uint16_t dev_id = mdio_read(3);
        if (org_id != 0xFFFF && dev_id != 0xFFFF) {
            printf("Found PHY at address %d: ID %04X%04X\n", phy_addr, org_id, dev_id);
            break;
        }
    }

    //SW reset
    mdio_write(0, 0x8000);
    while (mdio_read(0) & 0x8000) {} //wait for reset to complete


    uint16_t bcr = mdio_read(0); // BMCR
    // bcr |= (1 << 14); //enable loopback mode for testing
    bcr |= (1 << 9); //enable auto negotiation
    // bcr &= ~(1 << 12); //disable AN
    bcr |= (1 << 9);   // restart AN
        
    bcr |= (1 << 13); //100mbps
    bcr |= (1 << 8); //enable FD
    mdio_write(0, bcr);

    uint16_t bmsr;
    do {
        bmsr = mdio_read(1); // BMSR
    } while (!(bmsr & (1 << 5))); // wait for AN complete

    printf("ANLPAR (partner abilities): 0x%04X\n", mdio_read(5));
    printf("PHY special status (reg31): 0x%04X\n", mdio_read(31));
    printf("Special modes (reg18): 0x%04X\n", mdio_read(18));
    printf("basic control (reg0): 0x%04X\n", mdio_read(0));
}

void print_phy_status() {
    uint16_t reg = mdio_read(0);
    printf("PHY control reg: 0x%04X\n", reg);
    reg = mdio_read(1);
    printf("PHY status reg: 0x%04X\n", reg);
    reg = mdio_read(31);
    printf("PHY special control/status reg: 0x%04X\n", reg);
}
