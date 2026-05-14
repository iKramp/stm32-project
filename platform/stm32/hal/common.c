#include "common.h"
#include <stdint.h>
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

uint32_t log2_rounded_up(uint32_t value) {
    uint32_t result = 0;
    uint32_t temp = 1;
    while (temp < value) {
        temp <<= 1;
        result++;
    }
    return result;
}

void clean_cache(
        uint32_t set, 
        uint32_t way,
        uint32_t level,
        uint32_t num_sets,
        uint32_t associativity,
        uint32_t line_size
) {
    volatile uint32_t *SCB_DCISW = (uint32_t *)(0xE000EF60);
    uint32_t A = log2_rounded_up(associativity);
    uint32_t S = log2_rounded_up(num_sets);
    uint32_t L = log2_rounded_up(line_size);
    uint32_t B = L + S;
    uint32_t reg_val = 0;
    reg_val |= level << 1;
    reg_val |= way << (32 - A);
    reg_val |= set << L;
    *SCB_DCISW = reg_val;
}

void enable_caches() {
    volatile uint32_t *SCB_ICIALLU = (uint32_t *)(0xE000EF50);
    *SCB_ICIALLU = 0; // Invalidate entire I-Cache

    volatile uint32_t *SCB_CCR = (uint32_t *)(0xE000ED14);
    uint32_t ccr_val = *SCB_CCR;
    ccr_val |= (1 << 17); // Enable I-Cache
    *SCB_CCR = ccr_val;

    volatile uintptr_t *CSSELR = (uintptr_t *)0xE000ED84;
    volatile uint32_t *CCSIDR = (uint32_t *)0xE000ED80;
    volatile uint32_t *CLIDR = (uint32_t *)0xE000ED78;

    *CSSELR = 0;
    uint32_t ccsidr = *CCSIDR;
    uint32_t num_sets = ((ccsidr >> 13) & 0x7FFF) + 1;
    uint32_t associativity = ((ccsidr >> 3) & 0x3FF) + 1;
    uint32_t ways = associativity;
    uint32_t line_size_log = ((ccsidr & 0x7) + 2);
    uint32_t line_size = 1 << line_size_log;

    for (uint32_t way = 0; way < ways; way++) {
        for (uint32_t set = 0; set < num_sets; set++) {
            clean_cache(set, way, 0, num_sets, associativity, line_size * 4);
        }
    }

    //asm DSB ISB
    __asm volatile ("dsb":::"memory");
    __asm volatile ("isb":::"memory");

    printf("enabling D-Cache with %u sets, %u ways, line size %u bytes\n", num_sets, associativity, line_size);

    ccr_val |= (1 << 16); // Enable D-Cache
    *SCB_CCR = ccr_val;

    printf("Caches enabled\n");
}
