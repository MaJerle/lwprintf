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
    lwprintf_printf(&lw, "Char: %c\r\n", 'a');
    lwprintf_printf(&lw, "Unsigned bin: %b\r\n", (unsigned)28);
    lwprintf_printf(&lw, "Unsigned oct: %o\r\n", (unsigned)28);
    lwprintf_printf(&lw, "Unsigned dec: %u\r\n", (unsigned)28);
    lwprintf_printf(&lw, "Unsigned hex: %X %x\r\n", (unsigned)28, (unsigned)28);

    char buff[11] = { 0 };
    int n = lwprintf_snprintf(&lw, buff, sizeof(buff), "My text: %c, %u", 'a', (unsigned)123);
    printf("LEN: %d; STRLEN: %d, str: %s\r\n", (int)n, (int)strlen(buff), buff);

    return 0;
}
