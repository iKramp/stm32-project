#ifndef FB_TEXT_H
#define FB_TEXT_H

#include <stdint.h>

void write_text(const char *text);
void write_character(uint8_t c);
void scroll(void);

#endif // FB_TEXT_H
