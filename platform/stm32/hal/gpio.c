#include "gpio.h"
#include "register.h"
#define BASE_ADDR 0x58020000

uint8_t *get_port_reg(uint8_t port) {
    if (port>= 'a' && port<= 'k') {
        port= port- 'a';
    } else if (port>= 'A' && port<= 'K') {
        port= port- 'A';
    } else if (port> 'k' - 'a') { //raw numeric value in bounds
        return 0;
    }
    uint8_t *port_addr = (uint8_t *)BASE_ADDR + port * 0x400;
    return port_addr;
}

void set_moder(uint8_t port, uint8_t pin_num, uint8_t mode) {
    volatile uint32_t *port_reg = (uint32_t *)get_port_reg(port);
    if (port_reg == 0) {
        return;
    }

    set_register(port_reg , 0b11 << (pin_num * 2), (mode & 0b11) << (pin_num * 2));
}

void set_alternate(uint8_t port, uint8_t pin_num, uint8_t alt_func) {
    volatile uint32_t *port_reg = (uint32_t *)get_port_reg(port);
    if (port_reg == 0) {
        return;
    }

    set_moder(port, pin_num, MODER_ALTERNATE);

    uint32_t off = pin_num < 8 ? 0x20 : 0x24;
    volatile uint32_t *af_reg = port_reg + (off / 4);
    uint32_t bit_off = (pin_num % 8) * 4;
    set_register(af_reg, 0b1111 << bit_off, (alt_func & 0b1111) << bit_off);

}

void set_in_pull(uint8_t port, uint8_t pin_num, uint8_t pull) {
    volatile uint32_t *port_reg = (uint32_t *)(get_port_reg(port) + 0xC);
    if (port_reg == 0) {
        return;
    }

    set_register(port_reg , 0b11 << (pin_num * 2), (pull & 0b11) << (pin_num * 2));
}

void write_pin(uint8_t port, uint8_t pin_num, uint8_t value) {
    volatile uint32_t *port_reg = (uint32_t *)(get_port_reg(port) + 0x14);
    if (port_reg == 0) {
        return;
    }

#ifdef GPIO_USE_BSRR
    if (value) {
        *port_reg = 1 << pin_num; //set bit
    } else {
        *port_reg = 1 << (pin_num + 16); //reset bit
    }
#else
    uint32_t mask = 1 << pin_num;
    if (value) {
        *port_reg |= mask;
    } else {
        *port_reg &= ~mask;
    }
#endif
}

uint8_t read_pin(uint8_t port, uint8_t pin_num) {
    volatile uint32_t *port_reg = (uint32_t *)(get_port_reg(port) + 0x10);
    if (port_reg == 0) {
        return 0;
    }

    uint32_t mask = 1 << pin_num;
    return ((*port_reg & mask) != 0) ? 1 : 0;
}

void set_speed(uint8_t port, uint8_t pin_num, uint8_t speed) {
    volatile uint32_t *port_reg = (uint32_t *)(get_port_reg(port) + 0x8);
    if (port_reg == 0) {
        return;
    }

    set_register(port_reg , 0b11 << (pin_num * 2), (speed & 0b11) << (pin_num * 2));
}
