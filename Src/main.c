#include "gpio.h"
#include "clock.h"
#include "fmc.c"
#include <stdint.h>

#if !defined(__SOFT_FP__) && defined(__ARM_FP)
#warning
    "FPU is not initialized, but the project is compiling for an FPU. Please initialize the FPU before use."
#endif


int main(void) {
  init_clock();

  wait_micros(10000);

  init_sdram();

  wait_micros(10000);

  volatile uint32_t *addr = (uint32_t *)0xD0000000;
  // //do a funny
  *addr = 0xBEEF;
  uint32_t res = *addr;
  if (res != 0xBEEF) {
      while (1); // Error
  }

  // PI13 = output (LED)
  set_output('I', 13);

  while (1) {
      write_pin('I', 13, read_pin('I', 13) ^ 1);
      wait_micros(1000000);
  }
}
