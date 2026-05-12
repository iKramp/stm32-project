#include "fb_text.h"
#include "framebuffer.h"
#include "font.h"
#include <string.h>
#include "../../../include/draw.h"

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
    .width_chars = 30,  // Example: 480 pixels / 8 pixels per character
    .line = 30,
    .char_pos = 0
};

int _write(int file, const char *ptr, int len) {
    write_text(ptr, len);
    return len;
}

void set_foreground_color(uint8_t r, uint8_t g, uint8_t b) {
    fb_text_info.foreground_r = r;
    fb_text_info.foreground_g = g;
    fb_text_info.foreground_b = b;
}

void set_background_color(uint8_t r, uint8_t g, uint8_t b) {
    fb_text_info.background_r = r;
    fb_text_info.background_g = g;
    fb_text_info.background_b = b;
}

void do_newline(void);

void set_cursor_position(uint32_t line, uint32_t char_pos) {
    if (line < fb_text_info.height_lines) {
        fb_text_info.line = line;
    }
    if (char_pos < fb_text_info.width_chars) {
        fb_text_info.char_pos = char_pos;
    }
}

void display_bytes(const uint8_t *data, uint32_t length) {
    for (uint32_t i = 0; i < length; i++) {
        uint8_t byte = data[i];
        uint8_t high_nibble = (byte >> 4) & 0x0F;
        uint8_t low_nibble = byte & 0x0F;
        write_character(high_nibble < 10 ? ('0' + high_nibble) : ('A' + high_nibble - 10));
        write_character(low_nibble < 10 ? ('0' + low_nibble) : ('A' + low_nibble - 10));
        write_character(' ');
        if ((i + 1) % 16 == 0) {
            do_newline();
        }
    }
}

void write_text(const char *text, uint32_t len) {
    while (*text && len--) {
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

    uint32_t bytes_per_pixel = fb_info->bpp;
    uint32_t pitch = fb_info->width * bytes_per_pixel;

    // Width of the text region in pixels
    uint32_t text_width_pixels =
        fb_text_info.width_chars * CHAR_WIDTH;

    // Bytes to copy per scanline within text region
    uint32_t text_row_bytes =
        text_width_pixels * bytes_per_pixel;

    // Scroll height in pixels
    uint32_t scroll_height =
        (fb_text_info.height_lines - 1) * CHAR_HEIGHT;

    // Move text area upward
    for (uint32_t y = 0; y < scroll_height; y++) {
        volatile uint8_t *dst =
            fb + (y * pitch);

        volatile uint8_t *src =
            fb + ((y + CHAR_HEIGHT) * pitch);

        memcpy((void*)dst, (void*)src, text_row_bytes);
    }

    // Clear only the last text line
    draw_rectangle(
        0,
        scroll_height,
        text_width_pixels,
        CHAR_HEIGHT,
        0xFF000000
    );
}
