#include <stdio.h>
#include "lwprintf/lwprintf.h"
#include <string.h>
#include <stdint.h>

int
lwprintf_output(int ch, struct lwprintf* lw) {
    printf("%c", (char)ch);
    return ch;
}

lwprintf_t lw;

int
main(void) {

    lwprintf_init(&lw, lwprintf_output);
    lwprintf_printf(&lw, "Number: %d\r\n", 13);
    return 0;
}
