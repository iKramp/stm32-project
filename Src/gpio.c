#include "gpio.h"

uint32_t get_class_reg(uint8_t gpio_class) {
    const uint32_t BASE_ADDR = 0x58020000;
    if (gpio_class >= 'a' && gpio_class <= 'k') {
        gpio_class = gpio_class - 'a';
    } else if (gpio_class >= 'A' && gpio_class <= 'K') {
        gpio_class = gpio_class - 'A';
    } else if (gpio_class > 'k' - 'a') {
        return 0;
    }
    uint32_t class_addr = BASE_ADDR + gpio_class * 0x400;
    return class_addr;
}

void set_output(uint8_t gpio_class, uint8_t pin_num) {
    volatile uint32_t *class_reg = (uint32_t *)get_class_reg(gpio_class);
    if (class_reg == 0) {
        return;
    }

    uint32_t mask = 0b11 << (pin_num * 2);
    *class_reg &= ~mask;
    *class_reg |= 0b01 << pin_num * 2;
}

void set_input(uint8_t gpio_class, uint8_t pin_num) {
    volatile uint32_t *class_reg = (uint32_t *)get_class_reg(gpio_class);
    if (class_reg == 0) {
        return;
    }

    uint32_t mask = 0b11 << (pin_num * 2);
    *class_reg &= ~mask;
}

void set_alternate(uint8_t gpio_class, uint8_t pin_num, uint8_t alt_func) {
    volatile uint32_t *class_reg = (uint32_t *)get_class_reg(gpio_class);
    if (class_reg == 0) {
        return;
    }

    uint32_t mask = 0b11 << (pin_num * 2);
    *class_reg &= ~mask;
    *class_reg |= 0b10 << pin_num * 2;

    uint32_t off = pin_num < 8 ? 0x20 : 0x24;
    volatile uint32_t *af_reg = (uint32_t *)(get_class_reg(gpio_class) + off);
    uint32_t bit_off = (pin_num % 8) * 4;
    *af_reg &= ~(0b1111 << bit_off);
    *af_reg |= (alt_func & 0b1111) << bit_off;

}

const uint8_t PULL_FLOAT = 0;
const uint8_t PULL_UP = 1;
const uint8_t PULL_DOWN = 2;


void set_in_pull(uint8_t gpio_class, uint8_t pin_num, uint8_t pull) {
    volatile uint32_t *class_reg = (uint32_t *)(get_class_reg(gpio_class) + 0xC);
    if (class_reg == 0) {
        return;
    }

    uint32_t mask = 0b11 << (pin_num * 2);
    *class_reg &= ~mask;
    *class_reg |= (pull & 0b11) << (pin_num * 2);
}

void write_pin(uint8_t gpio_class, uint8_t pin_num, uint8_t value) {
    volatile uint32_t *class_reg = (uint32_t *)(get_class_reg(gpio_class) + 0x14);
    if (class_reg == 0) {
        return;
    }

    uint32_t mask = 1 << pin_num;
    if (value) {
        *class_reg |= mask;
    } else {
        *class_reg &= ~mask;
    }
}

uint8_t read_pin(uint8_t gpio_class, uint8_t pin_num) {
    volatile uint32_t *class_reg = (uint32_t *)(get_class_reg(gpio_class) + 0x10);
    if (class_reg == 0) {
        return 0;
    }

    uint32_t mask = 1 << pin_num;
    return ((*class_reg & mask) != 0) ? 1 : 0;
}

void set_speed(uint8_t gpio_class, uint8_t pin_num, uint8_t speed) {
    volatile uint32_t *class_reg = (uint32_t *)(get_class_reg(gpio_class) + 0x8);
    if (class_reg == 0) {
        return;
    }

    uint32_t mask = 0b11 << (pin_num * 2);
    *class_reg &= ~mask;
    *class_reg |= (speed & 0b11) << (pin_num * 2);
}
