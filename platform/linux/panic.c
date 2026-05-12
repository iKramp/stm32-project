#include "../../include/panic.h"
#include <stdio.h>
#include <stdlib.h>

void panic(const char *message) {
    printf("PANIC: %s\n", message);
    exit(1);
}
