#ifndef FB_TEXT_H
#define FB_TEXT_H

#include <stdint.h>

void write_text(const char *text);
void write_character(uint8_t c);
void scroll(void);
void display_bytes(const uint8_t *data, uint32_t length);
void set_cursor_position(uint32_t line, uint32_t char_pos);

#endif // FB_TEXT_H
