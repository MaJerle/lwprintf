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
 * \param[in]       expected: Expected result
 * \param[in]       fmt: Format to use
 * \param[in]       ...: Optional parameters
 */
static void
printf_run(const char* expected, const char* fmt, ...) {
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

    /* Float tests */
    for (float a = 0.0f; a < 1.0f; a += 0.01f) {
        printf_run(NULL, "%10f; %10.1f; %10.0f", 1.99f + a, 1.99f + a, 1.99f + a);
    }
    return 0;

    additional_format_specifiers();

    printf_run(" 28",               "% 3u", (unsigned)28);
    printf_run("123456",            "% 3u", (unsigned)123456);
    printf_run("028",               "%03d", (unsigned)28);
    printf_run("+28",               "%+03d", (unsigned)28);
    printf_run("+28",               "%+3d", (unsigned)28);
    printf_run("-28",               "%03d", -28);
    printf_run(NULL, "%+03d", -28);
    printf_run(NULL, "%+3d", -28);
    printf_run(NULL, "%03u", (unsigned)123456);
    printf_run(NULL, "%-010uabc", (unsigned)123456);
    printf_run(NULL, "%010uabc", (unsigned)123456);
    printf_run(NULL, "%-10d", -123);
    printf_run(NULL, "%10d", -123);
    printf_run(NULL, "%-06d", -1234567);
    printf_run(NULL, "%06d", -1234567);
    printf_run(NULL, "%s", "This is my string");
    printf_run(NULL, "%10s", "This is my string");
    printf_run(NULL, "%0*d", 10, -123);
    printf_run(NULL, "%zu", (size_t)10);
    printf_run(NULL, "%ju", (uintmax_t)10);
    printf_run(NULL, "% d", 1024);
    printf_run(NULL, "% 4d", 1024);
    printf_run(NULL, "% 3d", 1024);
    printf_run(NULL, "% 3f", 32.687);

    /* string */
    printf_run(NULL, "%*.*s", 8, 12, "This is my string");
    printf_run(NULL, "%*.*s", 8, 12, "Stri");
    printf_run(NULL, "%-6.10s", "This is my string");
    printf_run(NULL, "%6.10s", "This is my string");
    printf_run(NULL, "%-6.10s", "This is my string");
    printf_run(NULL, "%6.10s", "Th");
    printf_run(NULL, "%-6.10s", "Th");
    printf_run(NULL, "%*.*s", -6, 10, "Th");
    printf_run(NULL, "%*.*s", 6, 10, "Th");

    printf_run(NULL, "%.4s", "This is my string");
    printf_run(NULL, "%.6s", "1234");
    printf_run(NULL, "%.4s", "stri");
    printf_run(NULL, "%.4s%.2s", "123456", "abcdef");
    printf_run(NULL, "%.4.2s", "123456");
    printf_run(NULL, "%.*s", 3, "123456");
    printf_run(NULL, "%.3s", "");
    printf_run(NULL, "%yunknown");

    /* Alternate form */
    printf_run(NULL, "%#2X", 123);
    printf_run(NULL, "%#2x", 123);
    printf_run(NULL, "%#2o", 123);
    printf_run(NULL, "%#2X", 1);
    printf_run(NULL, "%#2x", 1);
    printf_run(NULL, "%#2o", 1);
    printf_run(NULL, "%#2X", 0);
    printf_run(NULL, "%#2x", 0);
    printf_run(NULL, "%#2o", 0);

    /* Pointers */
    printf_run(NULL, "%p", &tests_passed);
    printf_run(NULL, "0X%p", &tests_passed);
    printf_run(NULL, "0x%p", &tests_passed);

    /* Print final output */
    printf("\r\n\r\n------------------------\r\n\r\n");
    printf("Number of tests passed: %d\r\n", (int)tests_passed);
    printf("Number of tests failed: %d\r\n", (int)tests_failed);
    printf("Coverage: %f %%\r\n", (float)((tests_passed * 100) / ((float)(tests_passed + tests_failed))));
    printf("\r\n\r\n------------------------\r\n\r\n");

    /* Those are additional, not supported in classic printf implementation */

    /* Binary */
    printf_run(NULL, "%llb abc", 123);
    printf_run(NULL, "%llb abc", 123);
    printf_run(NULL, "%b", 4);
    printf_run(NULL, "%#2B", 1);
    printf_run(NULL, "%#2b", 1);
    printf_run(NULL, "%#2B", 0);
    printf_run(NULL, "%#2b", 0);
    printf_run(NULL, "%#B", 0);
    printf_run(NULL, "%#b", 0);
    printf_run(NULL, "%#B", 6);
    printf_run(NULL, "%#b", 6);

    /* Array test */
    uint8_t my_arr[] = { 0x00, 0x01, 0xB5, 0xC6, 0xD7 };
    printf_run("0102B5C6D7", "%5K", my_arr);
    printf_run("0102B5", "%*K", 3, my_arr);
    printf_run("01 02 B5", "% *K", 3, my_arr);
    printf_run("0102b5c6d7", "%5k", my_arr);
    printf_run("0102b5", "%*k", 3, my_arr);
    printf_run("01 02 b5", "% *k", 3, my_arr);

    return 0;
}
