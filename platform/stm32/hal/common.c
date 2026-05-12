#include "common.h"
#include <string.h>
#include <stdio.h>

void panic(const char *message) {
    uint32_t len = strlen(message);
    set_foreground_color(255, 0, 0); // Red text
    set_background_color(0, 0, 0); // Black background
    write_text("PANIC: ", 7);
    write_text(message, len);
    while (1);
}

uint32_t get_uid() {
    uint32_t uid[3];
    volatile uint32_t *UID_BASE = (uint32_t *)0x1FF1E800;
    uid[0] = UID_BASE[0];
    uid[1] = UID_BASE[1];
    uid[2] = UID_BASE[2];
    return uid[0] ^ uid[1] ^ uid[2];
}

void print_cache_info() {
    volatile uint32_t *CCSIDR = (uint32_t *)0xE000ED80;
    volatile uint32_t *CLIDR = (uint32_t *)0xE000ED78;

    uint32_t clidr = *CLIDR;
    uint32_t ccsidr = *CCSIDR;

    printf("CLIDR: 0x%08X\n", clidr);
    printf("CCSIDR: 0x%08X\n", ccsidr);

    #define SCB_BASE 0xE000ED10UL

    // SCB registers
    volatile uint32_t *SCB_CCR = (uint32_t *)(SCB_BASE + 0x14);
    // CCR bits
    #define CCR_DC_BIT (1 << 17) // Data Cache Enable
    #define CCR_IC_BIT (1 << 16) // Instruction Cache Enable

    uint32_t ccr_val = *SCB_CCR;
    printf("Cache Configuration:\n");
    if (ccr_val & CCR_IC_BIT) {
        printf("  - Instruction Cache (I-Cache): ENABLED\n");
    } else {
        printf("  - Instruction Cache (I-Cache): DISABLED\n");
    }

    if (ccr_val & CCR_DC_BIT) {
        printf("  - Data Cache (D-Cache): ENABLED\n");
    } else {
        printf("  - Data Cache (D-Cache): DISABLED\n");
    }
}
