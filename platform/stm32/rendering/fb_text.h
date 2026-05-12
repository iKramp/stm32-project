#ifndef FB_TEXT_H
#define FB_TEXT_H

#include <stdint.h>

int _write(int file, const char *ptr, int len);
void write_text(const char *text, uint32_t len);
void write_character(uint8_t c);
void scroll(void);
void display_bytes(const uint8_t *data, uint32_t length);
void set_cursor_position(uint32_t line, uint32_t char_pos);
void set_foreground_color(uint8_t r, uint8_t g, uint8_t b);
void set_background_color(uint8_t r, uint8_t g, uint8_t b);

#endif // FB_TEXT_H
