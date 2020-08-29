#include <stdio.h>
#include "lwprintf/lwprintf.h"
#include <string.h>
#include <stdint.h>
#include "windows.h"

/**
 * \brief           Output function for lwprintf printf function
 * \param[in]       ch: Character to print
 * \param[in]       lw: LwPRINTF instance
 * \return          `ch` value on success, `0` otherwise
 */
int
lwprintf_output(int ch, lwprintf_t* lw) {
    printf("%c", (char)ch);
    return ch;
}

/**
 * \brief           Tests numbers
 */
size_t tests_passed, tests_failed;

/**
 * \brief           Run printf with built-in and custom implementation.
 * Compare results on returned length and actual content
 *
 * \param[in]       fmt: Format to use
 * \param[in]       ...: Optional parameters
 */
static void
printf_run(const char* fmt, ...) {
    HANDLE console;
    va_list va;
    char b1[255] = { 0 }, b2[255] = { 0 };
    int l1, l2;

    console = GetStdHandle(STD_OUTPUT_HANDLE);  /* Get console */

    /* Generate both strings with original and custom printf */
    va_start(va, fmt);
    l1 = vsnprintf(b1, sizeof(b1), fmt, va);
    l2 = lwprintf_vsnprintf(b2, sizeof(b2), fmt, va);
    va_end(va);

    printf("Format: \"%s\"\r\n", fmt);
    printf("R: Len: %3d, result: \"%s\"\r\n", l1, b1);
    printf("L: Len: %3d, result: \"%s\"\r\n", l2, b2);
    if (strcmp(b1, b2) || l1 != l2) {
        SetConsoleTextAttribute(console, FOREGROUND_RED);
        printf("Test failed!\r\n");
        ++tests_failed;
    } else {
        SetConsoleTextAttribute(console, FOREGROUND_GREEN);
        printf("Test passed!\r\n");
        ++tests_passed;
    }
    SetConsoleTextAttribute(console, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    printf("----\r\n");
}

int n;
int
main(void) {
    double num = 2123213213142.032;

    lwprintf_init(lwprintf_output);

    printf("%lf\r\n", num);
    printf("%lf\r\n", num - (double)((long)(num)));

    printf_run("% 3u", (unsigned)28);
    printf_run("% 3u", (unsigned)123456);
    printf_run("%03d", (unsigned)28);
    printf_run("%+03d", (unsigned)28);
    printf_run("%+3d", (unsigned)28);
    printf_run("%03d", -28);
    printf_run("%+03d", -28);
    printf_run("%+3d", -28);
    printf_run("%03u", (unsigned)123456);
    printf_run("%-010uabc", (unsigned)123456);
    printf_run("%010uabc", (unsigned)123456);
    printf_run("%-10d", -123);
    printf_run("%10d", -123);
    printf_run("%-06d", -1234567);
    printf_run("%06d", -1234567);
    printf_run("%s", "This is my string");
    printf_run("%10s", "This is my string");
    printf_run("%0*d", 10, -123);
    printf_run("%zu", (size_t)10);
    printf_run("%ju", (uintmax_t)10);

    /* string */
    printf_run("%*.*s", 8, 12, "This is my string");
    printf_run("%*.*s", 8, 12, "Stri");
    printf_run("%-6.10s", "This is my string");
    printf_run("%6.10s", "This is my string");
    printf_run("%-6.10s", "This is my string");
    printf_run("%6.10s", "Th");
    printf_run("%-6.10s", "Th");
    printf_run("%*.*s", -6, 10, "Th");
    printf_run("%*.*s", 6, 10, "Th");

    printf_run("%.4s", "This is my string");
    printf_run("%.6s", "1234");
    printf_run("%.4s", "stri");
    printf_run("%.4s%.2s", "123456", "abcdef");
    printf_run("%.4.2s", "123456");
    printf_run("%.*s", 3, "123456");
    printf_run("%.3s", "");
    printf_run("%yunknown");

    /* Alternate form */
    printf_run("%#2X", 123);
    printf_run("%#2x", 123);
    printf_run("%#2o", 123);
    printf_run("%#2X", 1);
    printf_run("%#2x", 1);
    printf_run("%#2o", 1);
    printf_run("%#2X", 0);
    printf_run("%#2x", 0);
    printf_run("%#2o", 0);

    /* Pointers */
    printf_run("%p", &tests_passed);
    printf_run("0X%p", &tests_passed);
    printf_run("0x%p", &tests_passed);

    /* Binary */
    printf_run("%llb abc", 123);
    printf_run("%llb abc", 123);
    printf_run("%b", 4);

    /* Floats */
    printf_run("%f", 12.13);
    printf_run("%.3f", 12.1337);
    printf_run("%.25f", 12.1337);

    /* Engineering */
    printf_run("%e", 2.5f);
    printf_run("%e", 43433.23f);

    /* A */
    printf_run("%a", 2.5f);
    printf_run("%a", 43433.23f);

    /* Array test */
    uint8_t my_arr[] = { 0x00, 0x01, 0x02, 0x03, 0x04 };
    printf_run("%5K", my_arr);              /* Print fixed length */
    printf_run("%*K", 3, my_arr);           /* Print only first 3 elements of array */
    printf_run("% *K", 3, my_arr);          /* Print only first 3 elements of array with spaces between bytes */

    /* Print final output */
    printf("----\r\n\r\n");
    printf("Number of tests passed: %d\r\n", (int)tests_passed);
    printf("Number of tests failed: %d\r\n", (int)tests_failed);
    printf("Coverage: %f %%\r\n", (float)((tests_passed) / ((float)(tests_passed + tests_failed)) * 100));
    printf("----\r\n\r\n");

    return 0;
}
