#ifndef PHY_H
#define PHY_H

#include <stdint.h>

uint16_t mdio_read(uint8_t reg_addr);
void mdio_write(uint8_t reg_addr, uint16_t data);
void init_phy();
void print_phy_status();

#endif // PHY_H
