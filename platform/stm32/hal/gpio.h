#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>

#define MODER_INPUT 0b00
#define MODER_OUTPUT 0b01
#define MODER_ALTERNATE 0b10
#define MODER_ANALOG 0b11

#define PULL_FLOAT 0b00
#define PULL_UP 0b01
#define PULL_DOWN 0b10

uint8_t *get_port_reg(uint8_t port);
void set_moder(uint8_t port, uint8_t pin_num, uint8_t mode);
void set_alternate(uint8_t port, uint8_t pin_num, uint8_t alt_func);
void set_in_pull(uint8_t port, uint8_t pin_num, uint8_t pull);
uint8_t read_pin(uint8_t port, uint8_t pin_num);
void write_pin(uint8_t port, uint8_t pin_num, uint8_t value);
void set_speed(uint8_t port, uint8_t pin_num, uint8_t speed);

#endif // GPIO_H
