#include "gpio.h"
#include "init_clock.c"
#include "fmc.c"
#include <stdint.h>

#if !defined(__SOFT_FP__) && defined(__ARM_FP)
#warning
    "FPU is not initialized, but the project is compiling for an FPU. Please initialize the FPU before use."
#endif

void wait(volatile uint32_t count) {
  while (count--) {
    __asm__("nop");
  }
}

//9.8s per flash

int main(void) {
  init_clock();

  wait(1000000);

  init_sdram();

  wait(100000000);
 
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
      wait(10000000);
  }
}
