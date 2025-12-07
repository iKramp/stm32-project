#include "fb_text.h"
#include "framebuffer.h"
#include "font.h"

struct FrameBufferTextInfo {
    uint8_t foreground_r;
    uint8_t foreground_g;
    uint8_t foreground_b;
    uint8_t background_r;
    uint8_t background_g;
    uint8_t background_b;
    uint32_t height_lines;
    uint32_t width_chars;
    uint32_t line;
    uint32_t char_pos;
};

static struct FrameBufferTextInfo fb_text_info = {
    .foreground_r = 255,
    .foreground_g = 255,
    .foreground_b = 255,
    .background_r = 0,
    .background_g = 0,
    .background_b = 0,
    .height_lines = 34, // Example: 272 pixels / 8 pixels per character
    .width_chars = 60,  // Example: 480 pixels / 8 pixels per character
    .line = 30,
    .char_pos = 0
};

void do_newline(void);

void write_text(const char *text) {
    while (*text) {
        if (*text == '\n') {
            do_newline();
        } else {
            write_character((uint8_t)(*text));
        }
        text++;
    }
}

void write_character(uint8_t c) {
    if (c < 0x20 || c >= 0x7F) {
        c = 0xfe;
    }

    uint8_t *font = get_default_font();
    uint8_t *char_bitmap = font + (c * 8);
    uint32_t curr_row = fb_text_info.line * CHAR_HEIGHT;
    for (int line_index = 0; line_index < 8; line_index++) {
        uint8_t line = char_bitmap[line_index];
        for (int bit = 0; bit < 8; bit++) {
            uint8_t pixel_on = (line & (128 >> bit)) != 0;
            uint32_t color = pixel_on ?
                (0xFF << 24) | (fb_text_info.foreground_r << 16) | (fb_text_info.foreground_g << 8) | (fb_text_info.foreground_b) :
                (0xFF << 24) | (fb_text_info.background_r << 16) | (fb_text_info.background_g << 8) | (fb_text_info.background_b);
            draw_pixel(fb_text_info.char_pos * CHAR_WIDTH + bit, curr_row, color);
        };
        curr_row++;
    }

    fb_text_info.char_pos++;
    if (fb_text_info.char_pos >= fb_text_info.width_chars) {
        do_newline();
    }
}

void do_newline() {
    fb_text_info.char_pos = 0;
    fb_text_info.line++;
    if (fb_text_info.line > fb_text_info.height_lines - 1) {
        fb_text_info.line = fb_text_info.height_lines - 1;
        scroll();
    }
}

void scroll() {
    struct FrameBuffer* fb_info = get_fb();
    volatile uint8_t *fb = fb_info->buffer;
    uint32_t diff = fb_info->width * fb_info->bpp * CHAR_HEIGHT;
    volatile uint8_t *limit = fb + (diff * (fb_text_info.height_lines - 1) - 1);

    for (volatile uint8_t *ptr = fb; ptr <= limit; ptr += 4)
        *(volatile uint32_t*)ptr = *(volatile uint32_t*)(ptr + diff);

    // Clear the last line
    uint32_t start_of_last_line = diff * (fb_text_info.height_lines - 1);
    draw_rectangle(0, fb_info->height - CHAR_HEIGHT, fb_info->width, CHAR_HEIGHT, 0xFF000000);

}
