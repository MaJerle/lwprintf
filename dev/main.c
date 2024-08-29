#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "lwprintf/lwprintf.h"
#include "windows.h"

/**
 * \brief           Output function for lwprintf printf function
 * \param[in]       ch: Character to print
 * \param[in]       lw: LwPRINTF instance
 * \return          `ch` value on success, `0` otherwise
 */
int
lwprintf_output(int ch, lwprintf_t* lw) {
    LWPRINTF_UNUSED(lw);
    if (ch != '\0') {
        printf("%c", (char)ch);
    }
    return ch;
}

/**
 * \brief           Tests numbers
 */
size_t tests_passed, tests_failed;

#define do_test(buff_ptr, buff_size, exp_out, exp_out_len, fmt, ...)                                                   \
    do {                                                                                                               \
        char my_buffer[1234];                                                                                          \
        int len = lwprintf_snprintf((buff_ptr), (buff_size), (fmt), ##__VA_ARGS__);                                    \
        if (len != (exp_out_len)) {                                                                                    \
            printf("Test error on line: %d\r\n", __LINE__);                                                            \
            printf("Exp output len: %d, actual len: %d\r\n", (int)(exp_out_len), len);                                 \
            tests_failed++;                                                                                            \
        } else if ((buff_ptr) != NULL && strcmp(buff_ptr, exp_out) != 0) {                                             \
            printf("Test error on line: %d\r\n", __LINE__);                                                            \
            printf("Buffers do not match, expected: \"%s\", actual: \"%s\"\r\n", exp_out, buff_ptr);                   \
            tests_failed++;                                                                                            \
        } else {                                                                                                       \
            tests_passed++;                                                                                            \
        }                                                                                                              \
                                                                                                                       \
    } while (0)

int
main(void) {
    double num = 2123213213142.032;
    char test[123];
    char buffer[1024];

    lwprintf_init(lwprintf_output);

    /* Good tests */
    do_test(buffer, sizeof(buffer), "               4e+08", 20, "%20.*g", 0, 432432423.342321321);
    do_test(buffer, sizeof(buffer), "               4e+08", 20, "%20.*g", 1, 432432423.342321321);
    do_test(buffer, sizeof(buffer), "             4.3e+08", 20, "%20.*g", 2, 432432423.342321321);
    do_test(buffer, sizeof(buffer), "            4.32e+08", 20, "%20.*g", 3, 432432423.342321321);
    do_test(buffer, sizeof(buffer), "           4.324e+08", 20, "%20.*g", 4, 432432423.342321321);
    do_test(buffer, sizeof(buffer), "          4.3243e+08", 20, "%20.*g", 5, 432432423.342321321);
    do_test(buffer, sizeof(buffer), "         4.32432e+08", 20, "%20.*g", 6, 432432423.342321321);
    do_test(buffer, sizeof(buffer), "        4.324324e+08", 20, "%20.*g", 7, 432432423.342321321);
    do_test(buffer, sizeof(buffer), "       4.3243242e+08", 20, "%20.*g", 8, 432432423.342321321);
    do_test(buffer, sizeof(buffer), "           432432423", 20, "%20.*g", 9, 432432423.342321321);
    do_test(buffer, sizeof(buffer), "         432432423.3", 20, "%20.*g", 10, 432432423.342321321);
    do_test(buffer, sizeof(buffer), "        432432423.34", 20, "%20.*g", 11, 432432423.342321321);
    do_test(buffer, sizeof(buffer), "       432432423.342", 20, "%20.*g", 12, 432432423.342321321);
    do_test(buffer, sizeof(buffer), "      432432423.3423", 20, "%20.*g", 13, 432432423.342321321);
    do_test(buffer, sizeof(buffer), "     432432423.34232", 20, "%20.*g", 14, 432432423.342321321);
    do_test(buffer, sizeof(buffer), "    432432423.342321", 20, "%20.*g", 15, 432432423.342321321);
    do_test(buffer, sizeof(buffer), "   432432423.3423213", 20, "%20.*g", 16, 432432423.342321321);
    do_test(buffer, sizeof(buffer), "  432432423.34232134", 20, "%20.*g", 17, 432432423.342321321);
    do_test(buffer, sizeof(buffer), " 432432423.342321336", 20, "%20.*g", 18, 432432423.342321321);

    do_test(buffer, sizeof(buffer), "              0.0001", 20, "%20.*g", 0, 0.0001234567);
    do_test(buffer, sizeof(buffer), "              0.0001", 20, "%20.*g", 1, 0.0001234567);
    do_test(buffer, sizeof(buffer), "             0.00012", 20, "%20.*g", 2, 0.0001234567);
    do_test(buffer, sizeof(buffer), "            0.000123", 20, "%20.*g", 3, 0.0001234567);
    do_test(buffer, sizeof(buffer), "           0.0001235", 20, "%20.*g", 4, 0.0001234567);
    do_test(buffer, sizeof(buffer), "          0.00012346", 20, "%20.*g", 5, 0.0001234567);
    do_test(buffer, sizeof(buffer), "         0.000123457", 20, "%20.*g", 6, 0.0001234567);
    do_test(buffer, sizeof(buffer), "        0.0001234567", 20, "%20.*g", 7, 0.0001234567);
    do_test(buffer, sizeof(buffer), "        0.0001234567", 20, "%20.*g", 8, 0.0001234567);
    do_test(buffer, sizeof(buffer), "        0.0001234567", 20, "%20.*g", 9, 0.0001234567);
    do_test(buffer, sizeof(buffer), "        0.0001234567", 20, "%20.*g", 10, 0.0001234567);
    do_test(buffer, sizeof(buffer), "        0.0001234567", 20, "%20.*g", 11, 0.0001234567);
    do_test(buffer, sizeof(buffer), "        0.0001234567", 20, "%20.*g", 12, 0.0001234567);
    do_test(buffer, sizeof(buffer), "        0.0001234567", 20, "%20.*g", 13, 0.0001234567);
    do_test(buffer, sizeof(buffer), "        0.0001234567", 20, "%20.*g", 14, 0.0001234567);
    do_test(buffer, sizeof(buffer), "        0.0001234567", 20, "%20.*g", 15, 0.0001234567);
    do_test(buffer, sizeof(buffer), "        0.0001234567", 20, "%20.*g", 16, 0.0001234567);
    do_test(buffer, sizeof(buffer), "        0.0001234567", 20, "%20.*g", 17, 0.0001234567);
    do_test(buffer, sizeof(buffer), "        0.0001234567", 20, "%20.*g", 18, 0.0001234567);
    do_test(buffer, sizeof(buffer), "        0.0001234567", 20, "%20.*g", 19, 0.0001234567);

    do_test(buffer, sizeof(buffer), "3.2332", 6, "%.4f", 3.23321321);
    do_test(buffer, sizeof(buffer), "1.23342", 7, "%g", 1.23342);
    do_test(buffer, sizeof(buffer), "12334.2", 7, "%g", 12334.2);
    do_test(buffer, sizeof(buffer), "1.23342e-07", 11, "%.8g", 0.000000123342);
    do_test(buffer, sizeof(buffer), "1.23342E-07", 11, "%.8G", 0.000000123342);
    do_test(buffer, sizeof(buffer), "323243432432432.4375", 20, "%.4f", 323243432432432.432);
    do_test(buffer, sizeof(buffer), "-1.234560e+02", 13, "%e", -123.456);
    do_test(buffer, sizeof(buffer), "1.000000e-06", 12, "%e", 0.000001);
    do_test(buffer, sizeof(buffer), "1.234560e-01", 12, "%e", 0.123456);
    do_test(buffer, sizeof(buffer), "-1.234560e-01", 13, "%e", -0.123456);
    do_test(buffer, sizeof(buffer), "1.2346e+02", 10, "%.4e", 123.456);
    do_test(buffer, sizeof(buffer), "-1.2346e+02", 11, "%.4e", -123.456);
    do_test(buffer, sizeof(buffer), "1.2346e-01", 10, "%.4e", 0.123456);
    do_test(buffer, sizeof(buffer), "-1.2346e-01", 11, "%.4e", -0.123456);
    do_test(buffer, sizeof(buffer), "1e+02", 5, "%.0e", 123.456);
    do_test(buffer, sizeof(buffer), "-1e+02", 6, "%.0e", -123.456);
    do_test(buffer, sizeof(buffer), "1e-01", 5, "%.0e", 0.123456);
    do_test(buffer, sizeof(buffer), "-1e-01", 6, "%.0e", -0.123456);
    do_test(buffer, sizeof(buffer), "            1.2346e+02", 22, "%22.4e", 123.456);
    do_test(buffer, sizeof(buffer), "           -1.2346e+02", 22, "%22.4e", -123.456);
    do_test(buffer, sizeof(buffer), "            1.2346e-01", 22, "%22.4e", 0.123456);
    do_test(buffer, sizeof(buffer), "           -1.2346e-01", 22, "%22.4e", -0.123456);
    do_test(buffer, sizeof(buffer), "0000000000001.2346e+02", 22, "%022.4e", 123.456);
    do_test(buffer, sizeof(buffer), "-000000000001.2346e+02", 22, "%022.4e", -123.456);
    do_test(buffer, sizeof(buffer), "0000000000001.2346e-01", 22, "%022.4e", 0.123456);
    do_test(buffer, sizeof(buffer), "1.234560e-09", 12, "%e", 0.00000000123456);
    do_test(buffer, sizeof(buffer), "-000000000001.2346e-01", 22, "%022.4e", -0.123456);
    do_test(buffer, sizeof(buffer), "-1.2346E+02", 11, "%.4E", -123.456);
    do_test(buffer, sizeof(buffer), " 28", 3, "% 3u", (unsigned)28);
    do_test(buffer, sizeof(buffer), "028", 3, "%03d", 28);
    do_test(buffer, sizeof(buffer), "+28", 3, "%+03d", 28);
    do_test(buffer, sizeof(buffer), "+28", 3, "%+3d", 28);
    do_test(buffer, sizeof(buffer), "-28", 3, "%03d", -28);
    do_test(buffer, sizeof(buffer), "-28", 3, "%+03d", -28);
    do_test(buffer, sizeof(buffer), "-28", 3, "%+3d", -28);
    do_test(buffer, sizeof(buffer), "123456", 6, "%03u", (unsigned)123456);
    do_test(buffer, sizeof(buffer), "123456    abc", 13, "%-010uabc", (unsigned)123456);
    do_test(buffer, sizeof(buffer), "0000123456abc", 13, "%010uabc", (unsigned)123456);
    do_test(buffer, sizeof(buffer), "-123      ", 10, "%-10d", -123);
    do_test(buffer, sizeof(buffer), "      -123", 10, "%10d", -123);
    do_test(buffer, sizeof(buffer), "-1234567", 8, "%-06d", -1234567);
    do_test(buffer, sizeof(buffer), "-1234567", 8, "%06d", -1234567);
    do_test(buffer, sizeof(buffer), "-1234567  ", 10, "%-10d", -1234567);
    do_test(buffer, sizeof(buffer), "  -1234567", 10, "%10d", -1234567);
    do_test(buffer, sizeof(buffer), "-1234567  ", 10, "%-010d", -1234567);
    do_test(buffer, sizeof(buffer), "-001234567", 10, "%010d", -1234567);
    do_test(buffer, sizeof(buffer), "-000000123", 10, "%0*d", 10, -123);
    do_test(buffer, sizeof(buffer), "10", 2, "%zu", (size_t)10);
    do_test(buffer, sizeof(buffer), "10", 2, "%ju", (uintmax_t)10);
    do_test(buffer, sizeof(buffer), " 1024", 5, "% d", 1024);
    do_test(buffer, sizeof(buffer), " 1024", 5, "% 4d", 1024);
    do_test(buffer, sizeof(buffer), " 1024", 5, "% 3d", 1024);
    do_test(buffer, sizeof(buffer), " 32.687000", 10, "% 3f", 32.687);

    /* Strings */
    do_test(buffer, sizeof(buffer), "", 0, "%.*s", 0, "Text string 123");
    do_test(buffer, sizeof(buffer), "T", 1, "%.*s", 1, "Text string 123");
    do_test(buffer, sizeof(buffer), "Te", 2, "%.*s", 2, "Text string 123");
    do_test(buffer, sizeof(buffer), "Tex", 3, "%.*s", 3, "Text string 123");
    do_test(buffer, sizeof(buffer), "Text", 4, "%.*s", 4, "Text string 123");
    do_test(buffer, sizeof(buffer), "Text ", 5, "%.*s", 5, "Text string 123");
    do_test(buffer, sizeof(buffer), "Text s", 6, "%.*s", 6, "Text string 123");
    do_test(buffer, sizeof(buffer), "Text st", 7, "%.*s", 7, "Text string 123");
    do_test(buffer, sizeof(buffer), "Text str", 8, "%.*s", 8, "Text string 123");
    do_test(buffer, sizeof(buffer), "Text stri", 9, "%.*s", 9, "Text string 123");
    do_test(buffer, sizeof(buffer), "Text strin", 10, "%.*s", 10, "Text string 123");
    do_test(buffer, sizeof(buffer), "Text string", 11, "%.*s", 11, "Text string 123");
    do_test(buffer, sizeof(buffer), "Text string ", 12, "%.*s", 12, "Text string 123");
    do_test(buffer, sizeof(buffer), "Text string 1", 13, "%.*s", 13, "Text string 123");
    do_test(buffer, sizeof(buffer), "Text string 12", 14, "%.*s", 14, "Text string 123");

    do_test(buffer, sizeof(buffer), "This is my string", 17, "%s", "This is my string");
    do_test(buffer, sizeof(buffer), "This is my string", 17, "%10s", "This is my string");
    do_test(buffer, sizeof(buffer), "This is my s", 12, "%*.*s", 8, 12, "This is my string");
    do_test(buffer, sizeof(buffer), "    Stri", 8, "%*.*s", 8, 12, "Stri");
    do_test(buffer, sizeof(buffer), "This is my", 10, "%-6.10s", "This is my string");
    do_test(buffer, sizeof(buffer), "This is my", 10, "%6.10s", "This is my string");
    do_test(buffer, sizeof(buffer), "This is my", 10, "%-6.10s", "This is my string");
    do_test(buffer, sizeof(buffer), "    Th", 6, "%6.10s", "Th");
    do_test(buffer, sizeof(buffer), "Th    ", 6, "%-6.10s", "Th");
    do_test(buffer, sizeof(buffer), "Th    ", 6, "%*.*s", -6, 10, "Th");
    do_test(buffer, sizeof(buffer), "    Th", 6, "%*.*s", 6, 10, "Th");
    do_test(buffer, sizeof(buffer), "This", 4, "%.4s", "This is my string");
    do_test(buffer, sizeof(buffer), "1234", 4, "%.6s", "1234");
    do_test(buffer, sizeof(buffer), "stri", 4, "%.4s", "stri");
    do_test(buffer, sizeof(buffer), "1234ab", 6, "%.4s%.2s", "123456", "abcdef");
    do_test(buffer, sizeof(buffer), "123", 3, "%.*s", 3, "123456");
    do_test(buffer, sizeof(buffer), "", 0, "%.3s", "");

    /* Hexadecimal */
    do_test(buffer, sizeof(buffer), "0X7B", 4, "%#2X", 123);
    do_test(buffer, sizeof(buffer), "0x7b", 4, "%#2x", 123);
    do_test(buffer, sizeof(buffer), "0173", 4, "%#2o", 123);
    do_test(buffer, sizeof(buffer), "0X1", 3, "%#2X", 1);
    do_test(buffer, sizeof(buffer), "0x1", 3, "%#2x", 1);
    do_test(buffer, sizeof(buffer), "01", 2, "%#2o", 1);
    do_test(buffer, sizeof(buffer), " 0", 2, "%#2X", 0);
    do_test(buffer, sizeof(buffer), " 0", 2, "%#2x", 0);
    do_test(buffer, sizeof(buffer), " 0", 2, "%#2o", 0);

    /* Pointer */
    void* my_pointer = (void*)0x12345678;
    do_test(buffer, sizeof(buffer), "12345678", 8, "%p", my_pointer);
    do_test(buffer, sizeof(buffer), "0X12345678", 10, "0X%p", my_pointer);
    do_test(buffer, sizeof(buffer), "0x12345678", 10, "0x%p", my_pointer);

    /* Binary data */
    do_test(buffer, sizeof(buffer), "1111011 abc", 11, "%llb abc", 123);
    do_test(buffer, sizeof(buffer), "100", 3, "%b", 4);
    do_test(buffer, sizeof(buffer), "0B1", 3, "%#2B", 1);
    do_test(buffer, sizeof(buffer), "0b1", 3, "%#2b", 1);
    do_test(buffer, sizeof(buffer), " 0", 2, "%#2B", 0);
    do_test(buffer, sizeof(buffer), " 0", 2, "%#2b", 0);
    do_test(buffer, sizeof(buffer), "0", 1, "%#B", 0);
    do_test(buffer, sizeof(buffer), "0", 1, "%#b", 0);
    do_test(buffer, sizeof(buffer), "0B110", 5, "%#B", 6);
    do_test(buffer, sizeof(buffer), "0b110", 5, "%#b", 6);

    /* Hex data */
    uint8_t my_arr[] = {0x01, 0x02, 0xB5, 0xC6, 0xD7};
    do_test(buffer, sizeof(buffer), "0102B5C6D7", 10, "%5K", my_arr);
    do_test(buffer, sizeof(buffer), "0102B5", 6, "%*K", 3, my_arr);
    do_test(buffer, sizeof(buffer), "01 02 B5", 8, "% *K", 3, my_arr);
    do_test(buffer, sizeof(buffer), "0102b5c6d7", 10, "%5k", my_arr);
    do_test(buffer, sizeof(buffer), "0102b5", 6, "%*k", 3, my_arr);
    do_test(buffer, sizeof(buffer), "01 02 b5", 8, "% *k", 3, my_arr);

    /* Length and data return */
    do_test(NULL, 0, "", 4, "test");
    do_test(buffer, sizeof(buffer), "Hello World!", 12, "Hello World!");

#if 0
    /* Problematic tests */
    do_test(buffer, sizeof(buffer), "0.000123456700005", 17, "%.*g", 17, 17, 0.0001234567);

    do_test(buffer, sizeof(buffer), "3.233213210000005056000000000000000000000000000", 47, "%.45f", 3.23321321);
    do_test(buffer, sizeof(buffer), "3.233213210000005056000000000000000000000000000", 47, "%.45F", 3.23321321);
    do_test(buffer, sizeof(buffer), "123456", 6, "% 3u", (unsigned)123456);
    do_test(buffer, sizeof(buffer), "1.234560000000005216000000000000000e+02", 39, "%22.33e", 123.456);
    do_test(buffer, sizeof(buffer), "-1.234560000000005216000000000000000e+02", 40, "%22.33e", -123.456);
    do_test(buffer, sizeof(buffer), "1.234560000000004992000000000000000e-01", 39, "%22.33e", 0.123456);
    do_test(buffer, sizeof(buffer), "-1.234560000000004992000000000000000e-01", 40, "%22.33e", -0.123456);
    do_test(buffer, sizeof(buffer), ".2s", 3, "%.4.2s", "123456");
    do_test(buffer, sizeof(buffer), "yunknown", 8, "%yunknown", "");
#endif

    printf("--------\r\n");
    printf("Tests passed: %u\r\n", (unsigned)(tests_passed));
    printf("Tests failed: %u\r\n", (unsigned)(tests_failed));
    printf("Tests total : %u\r\n", (unsigned)(tests_passed + tests_failed));
    printf("Coverage    : %f\r\n", (float)(tests_passed) / (float)(tests_passed + tests_failed));
}
