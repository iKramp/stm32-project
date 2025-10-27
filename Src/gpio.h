#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>

uint32_t get_class_reg(uint8_t gpio_class);
void set_output(uint8_t gpio_class, uint8_t pin_num);
void set_input(uint8_t gpio_class, uint8_t pin_num);
void set_alternate(uint8_t gpio_class, uint8_t pin_num, uint8_t alt_func);
void set_in_pull(uint8_t gpio_class, uint8_t pin_num, uint8_t pull);
uint8_t read_pin(uint8_t gpio_class, uint8_t pin_num);
void write_pin(uint8_t gpio_class, uint8_t pin_num, uint8_t value);
void set_speed(uint8_t gpio_class, uint8_t pin_num, uint8_t speed);

#endif // GPIO_H
