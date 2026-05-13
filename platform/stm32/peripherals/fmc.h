#ifndef FMC_H
#define FMC_H

#include <stdint.h>
#include "../hal/gpio.h"
#include "../hal/register.h"
#include "clock.h"
#include "../hal/common.h"

#define SDRAM_ADDR 0xD0000000
#define FB_ADDR                 SDRAM_ADDR //fb takes 480*272*4B of space
#define SCENE_DATA_ADDR         align_addr(SDRAM_ADDR + 480 * 272 * 4, 4096)


void init_sdram();

#endif // FMC_H
