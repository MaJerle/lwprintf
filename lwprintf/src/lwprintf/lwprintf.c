/**
 * \file            lwprintf.c
 * \brief           Lightweight stdio manager
 */

/*
 * Copyright (c) 2020 Tilen MAJERLE
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * This file is part of LwPRINTF - Lightweight stdio manager library.
 *
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 * Version:         $_version_$
 */
#include <limits.h>
#include "lwprintf/lwprintf.h"

#if LWPRINTF_CFG_OS
#include "system/lwprintf_sys.h"
#endif /* LWPRINTF_CFG_OS */

#define CHARISNUM(x)                    ((x) >= '0' && (x) <= '9')
#define CHARTONUM(x)                    ((x) - '0')

/**
 * \brief           Forward declaration
 */
struct lwprintf_int;

/**
 * \brief           Private output function declaration
 * \param[in]       lwi: Internal working structure
 * \param[in]       c: Character to print
 */
typedef int (*prv_output_fn)(struct lwprintf_int* p, const char c);

/**
 * \brief           Internal structure
 */
typedef struct lwprintf_int {
    lwprintf_t* lw;                             /*!< Instance handle */
    const char* fmt;                            /*!< Format string */
    char* const buff;                           /*!< Pointer to buffer when not using print option */
    const size_t buff_size;                     /*!< Buffer size of input buffer (when used) */
    int n;                                      /*!< Full length of formatted text */
    prv_output_fn out_fn;                       /*!< Output internal function */

    /* This must all be reset every time new % is detected */
    struct {
        struct {
            uint8_t left_align : 1;             /*!< Minus for left alignment */
            uint8_t plus : 1;                   /*!< Prepend + for positive numbers on the output */
            uint8_t space : 1;                  /*!< Prepend spaces. Not used with plus modifier */
            uint8_t zero : 1;                   /*!< Zero pad flag detection, add zeros if number length is less than width modifier */
            uint8_t thousands : 1;              /*!< Thousands has grouping applied */
            uint8_t alt : 1;                    /*!< Alternate form with hash */
            uint8_t precision : 1;              /*!< Precision flag has been used */

            /* Length modified flags */
            uint8_t longlong : 2;               /*!< Flag indicatin long-long number, used with 'l' (1) or 'll' (2) mode */
            uint8_t char_short : 2;             /*!< Used for 'h' (1 = short) or 'hh' (2 = char) length modifier */

            uint8_t uc : 1;                     /*!< Uppercase flag */
            uint8_t is_negative : 1;            /*!< Status if number is negative */
        } flags;                                /*!< List of flags */
        int precision;                          /*!< Selected precision */
        int width;                              /*!< Text width indicator */
        uint8_t base;                           /*!< Base for number format output */
    } m;                                        /*!< Block that is reset on every start of format */
} lwprintf_int_t;

/**
 * \brief           Get LwPRINTF instance based on user input
 * \param[in]       p: LwPRINTF instance.
 *                      Set to `NULL` for default instance
 */
#define LWPRINTF_GET_LW(p)              ((p) != NULL ? (p) : (&lwprintf_default))

/**
 * \brief           Output function for lwprintf printf function
 * \param[in]       ch: Character to print
 * \param[in]       lw: LwPRINTF instance
 * \return          `ch` value on success, `0` otherwise
 */
int
prv_default_output_func(int ch, struct lwprintf* lw) {
    LWPRINTF_UNUSED(ch);
    LWPRINTF_UNUSED(lw);
    return 0;
}

/**
 * \brief           LwPRINTF default structure used by application
 */
static lwprintf_t lwprintf_default = {
    .out = prv_default_output_func
};

/**
 * \brief           Rotate string of the input buffer in place
 * It rotates string from "abcdef" to "fedcba".
 *
 * \param[in,out]   str: Input and output string to be rotated
 * \param[in]       len: String length, optional parameter if
 *                      length is known in advance. Use `0` if not used
 * \return          `1` on success, `0` otherwise
 */
static int
prv_rotate_string(char* str, size_t len) {
    /* Get length if 0 */
    if (len == 0) {
        len = strlen(str);
    }

    /* Rotate string */
    for (size_t i = 0; i < len / 2; ++i) {
        char t = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = t;
    }

    return 1;
}

/**
 * \brief           Output function to print data
 * \param[in]       p: LwPRINTF internal instance
 * \param[in]       c: Character to print
 * \return          `1` on success, `0` otherwise
 */
static int
prv_out_fn_print(lwprintf_int_t* p, const char c) {
    p->lw->out(c, p->lw);                       /*!< Send character to output */
    ++p->n;
    return 1;
}

/**
 * \brief           Output function to generate buffer data
 * \param[in]       p: LwPRINTF internal instance
 * \param[in]       c: Character to write
 * \return          `1` on success, `0` otherwise
 */
static int
prv_out_fn_write_buff(lwprintf_int_t* p, const char c) {
    if (p->n < (p->buff_size - 1)) {
        p->buff[p->n++] = c;
        p->buff[p->n] = '\0';
        return 1;
    }
    return 0;
}

/**
 * \brief           Parse number from input string
 * \param[in,out]   format: Input text to process
 * \return          Parsed number
 */
static int
prv_parse_num(const char** format) {
    const char* fmt = *format;
    int n = 0;

    for (; CHARISNUM(*fmt); ++fmt) {
        n = 10 * n + CHARTONUM(*fmt);
    }
    *format = fmt;
    return n;
}

/**
 * \brief           Format data that are printed before actual value
 * \param[in,out]   p: LwPRINTF internal instance
 * \param[in]       buff_size: Expected buffer size of output string
 * \return          `1` on success, `0` otherwise
 */
static int
prv_out_str_before(lwprintf_int_t* p, size_t buff_size) {
    /* Check for width */
    if (p->m.width > 0 && p->m.flags.is_negative) {
        --p->m.width;
    }

    /* Check for alternate mode */
    if (p->m.flags.alt) {
        if (p->m.base == 8) {
            if (p->m.width > 0) {
                --p->m.width;
            }
        } else if (p->m.base == 16) {
            if (p->m.width > 2) {
                p->m.width -= 2;
            } else {
                p->m.width = 0;
            }
        }
    }

    /* Add negative sign before when zeros are used to fill width */
    if (p->m.flags.is_negative && p->m.flags.zero) {
        p->out_fn(p, '-');
    }

    /* Check for flags output */
    if (p->m.flags.alt) {
        if (p->m.base == 8) {
            p->out_fn(p, '0');
        } else if (p->m.base == 16) {
            p->out_fn(p, '0');
            p->out_fn(p, p->m.flags.uc ? 'X' : 'x');
        }
    }

    /* Right alignment, spaces or zeros */
    if (!p->m.flags.left_align && p->m.width > 0) {
        for (size_t i = buff_size; !p->m.flags.left_align && i < p->m.width; ++i) {
            p->out_fn(p, p->m.flags.zero ? '0' : ' ');
        }
    }

    /* Add negative sign here when spaces are used for width */
    if (p->m.flags.is_negative && !p->m.flags.zero) {
        p->out_fn(p, '-');
    }

    return 1;
}

/**
 * \brief           Format data that are printed after actual value
 * \param[in,out]   p: LwPRINTF internal instance
 * \param[in]       buff_size: Expected buffer size of output string
 * \return          `1` on success, `0` otherwise
 */
static int
prv_out_str_after(lwprintf_int_t* p, size_t buff_size) {
    /* Left alignment, but only with spaces */
    if (p->m.flags.left_align) {
        for (size_t i = buff_size; i < p->m.width; ++i) {
            p->out_fn(p, ' ');
        }
    }

    return 1;
}

/**
 * \brief           Output generate string from numbers/digits
 * Paddings before and after are applied at this stage
 *
 * \param[in,out]   p: LwPRINTF internal instance
 * \param[in]       buff: Buffer string
 * \param[in]       buff_size: Length of buffer to output
 * \return          `1` on success, `0` otherwise
 */
static int
prv_out_str(lwprintf_int_t* p, const char* buff, size_t buff_size) {
    /* Output string */
    if (buff_size == 0) {
        buff_size = strlen(buff);
    }

    /* Pre-value */
    prv_out_str_before(p, buff_size);

    /* Actual value */
    for (uint8_t i = 0; i < buff_size; ++i) {
        p->out_fn(p, buff[i]);
    }

    /* Post-value */
    prv_out_str_after(p, buff_size);

    return 1;
}

/**
 * \brief           Convert unsigned int to string
 * \param[in,out]   p: LwPRINTF internal instance
 * \param[in]       num: Number to convert to string
 * \return          `1` on success, `0` otherwise
 */
static int
prv_unsigned_int_to_str(lwprintf_int_t* p, unsigned int num) {
    unsigned int d, digit;
    size_t digits_cnt;
    char c;

    /* Get maximum number to start with */
    for (digits_cnt = 0, d = num; d > 0; ++digits_cnt, d /= p->m.base) {}
    for (d = 1; (num / d) >= p->m.base; d *= p->m.base) {}

    /* Print pre-number */
    prv_out_str_before(p, digits_cnt);
    for (; d > 0; ) {
        digit = num / d;
        num = num % d;
        d = d / p->m.base;
        c = (char)digit + (char)(digit >= 10 ? ((p->m.flags.uc ? 'A' : 'a') - 10) : '0');
        p->out_fn(p, c);
    }
    prv_out_str_after(p, digits_cnt);
    return 1;
}

/**
 * \brief           Convert unsigned long to string
 * \param[in,out]   p: LwPRINTF internal instance
 * \param[in]       num: Number to convert to string
 * \return          `1` on success, `0` otherwise
 */
static int
prv_unsigned_long_int_to_str(lwprintf_int_t* p, unsigned long int num) {
    unsigned long int d, digit;
    uint8_t digits_cnt;
    char c;

    /* Get maximum number to start with */
    for (digits_cnt = 0, d = num; d > 0; ++digits_cnt, d /= p->m.base) {}
    for (d = 1; (num / d) >= p->m.base; d *= p->m.base) {}

    /* Print pre-number */
    prv_out_str_before(p, digits_cnt);
    for (; d > 0; ) {
        digit = num / d;
        num = num % d;
        d = d / p->m.base;
        c = (char)digit + (char)(digit >= 10 ? ((p->m.flags.uc ? 'A' : 'a') - 10) : '0');
        p->out_fn(p, c);
    }
    prv_out_str_after(p, digits_cnt);
    return 1;
}

#if LWPRINTF_CFG_SUPPORT_LONG_LONG

/**
 * \brief           Convert unsigned long-long to string
 * \param[in,out]   p: LwPRINTF internal instance
 * \param[in]       num: Number to convert to string
 * \return          `1` on success, `0` otherwise
 */
static int
prv_unsigned_longlong_int_to_str(lwprintf_int_t* p, unsigned long long int num) {
    unsigned long long int d, digit;
    uint8_t digits_cnt;
    char c;

    /* Get maximum number to start with */
    for (digits_cnt = 0, d = num; d > 0; ++digits_cnt, d /= p->m.base) {}
    for (d = 1; (num / d) >= p->m.base; d *= p->m.base) {}

    /* Print pre-number */
    prv_out_str_before(p, digits_cnt);
    for (; d > 0; ) {
        digit = num / d;
        num = num % d;
        d = d / p->m.base;
        c = (char)digit + (char)(digit >= 10 ? ((p->m.flags.uc ? 'A' : 'a') - 10) : '0');
        p->out_fn(p, c);
    }
    prv_out_str_after(p, digits_cnt);
    return 1;
}

#endif /* LWPRINTF_CFG_SUPPORT_LONG_LONG */

/**
 * \brief           Convert signed int to string
 * \param[in,out]   p: LwPRINTF internal instance
 * \param[in]       num: Number to convert to string
 * \return          `1` on success, `0` otherwise
 */
static int
prv_signed_int_to_str(lwprintf_int_t* p, signed int num) {
    if (num < 0) {
        p->m.flags.is_negative = 1;
        num = -num;
    }
    return prv_unsigned_int_to_str(p, num);
}

/**
 * \brief           Convert signed long to string
 * \param[in,out]   p: LwPRINTF instance
 * \param[in]       num: Number to convert to string
 * \return          `1` on success, `0` otherwise
 */
static int
prv_signed_long_int_to_str(lwprintf_int_t* p, signed long int num) {
    if (num < 0) {
        p->m.flags.is_negative = 1;
        num = -num;
    }
    return prv_unsigned_long_int_to_str(p, num);
}

#if LWPRINTF_CFG_SUPPORT_LONG_LONG

/**
 * \brief           Convert signed long-long to string
 * \param[in,out]   p: LwPRINTF internal instance
 * \param[in]       num: Number to convert to string
 * \return          `1` on success, `0` otherwise
 */
static int
prv_signed_longlong_int_to_str(lwprintf_int_t* p, signed long long int num) {
    if (num < 0) {
        p->m.flags.is_negative = 1;
        num = -num;
    }
    return prv_unsigned_longlong_int_to_str(p, num);
}

#endif /* LWPRINTF_CFG_SUPPORT_LONG_LONG */

/**
 * \brief           Convert float number to string
 * \param[in,out]   p: LwPRINTF internal instance
 * \param[in]       num: Number to convert to string
 * \return          `1` on success, `0` otherwise
 */
static int
prv_float_to_str(lwprintf_int_t* p, float num) {
    return 0;
}

/**
 * \brief           Convert double number to string
 * \param[in,out]   p: LwPRINTF internal instance
 * \param[in]       num: Number to convert to string
 * \return          `1` on success, `0` otherwise
 */
static int
prv_double_to_str(lwprintf_int_t* p, double num) {
    return 0;
}

/**
 * \brief           Process format string and parse variable parameters
 * \param[in,out]   p: LwPRINTF internal instance
 * \param[in]       arg: Variable parameters list
 * \return          `1` on success, `0` otherwise
 */
static uint8_t
prv_format(lwprintf_int_t* p, va_list arg) {
    uint8_t detected = 0;
    const char* fmt = p->fmt;

    while (fmt != NULL && *fmt != '\0') {
        /* Parse format */
        /* %[parameter][flags][width][.precision][length]type */
        /* Go to https://en.wikipedia.org/wiki/Printf_format_string for more info */
        memset(&p->m, 0x00, sizeof(p->m));      /* Reset structure */

        /* Detect beginning */
        if (*fmt != '%') {
            p->out_fn(p, *fmt);                 /* Output character */
            ++fmt;
            continue;
        }
        ++fmt;

        /* Check [parameter] */
        /* Not used */

        /* Check [flags] */
        /* It can have multiple flags in any order */
        detected = 1;
        do {
            switch (*fmt) {
                case '-':   p->m.flags.left_align = 1;  break;
                case '+':   p->m.flags.plus = 1;        break;
                case ' ':   p->m.flags.space = 1;       break;
                case '0':   p->m.flags.zero = 1;        break;
                case '\'':  p->m.flags.thousands = 1;   break;
                case '#':   p->m.flags.alt = 1;         break;
                default:    detected = 0;               break;
            }
            if (detected) {
                ++fmt;
            }
        } while (detected);

        /* Check [width] */
        p->m.width = 0;
        if (CHARISNUM(*fmt)) {                  /* Fixed width check */
            /* If number is negative, it has been captured from previous step (left align) */
            p->m.width = prv_parse_num(&fmt);   /* Number from string directly */
        } else if (*fmt == '*') {               /* Or variable check */
            const int w = (int)va_arg(arg, int);
            if (w < 0) {
                p->m.flags.left_align = 1;      /* Negative width means left aligned */
                p->m.width = -w;
            } else {
                p->m.width = w;
            }
            ++fmt;
        }

        /* Check [.precision] */
        p->m.precision = 0;
        if (*fmt == '.') {                      /* Precision flag is detected */
            p->m.flags.precision = 1;
            ++fmt;
            if (*fmt == '*') {                  /* Variable check */
                const int pr = (int)va_arg(arg, int);
                p->m.precision = pr > 0 ? pr : 0;
                ++fmt;
            } else if (CHARISNUM(*fmt)) {       /* Directly in the string */
                p->m.precision = prv_parse_num(&fmt);
            }
        }

        /* Check [length] */
        detected = 1;
        switch (*fmt) {
            case 'h':
                p->m.flags.char_short = 1;      /* Single h detected */
                ++fmt;
                if (*fmt == 'h') {              /* Does it follow by another h? */
                    p->m.flags.char_short = 2;  /* Second h detected */
                    ++fmt;
                }
            case 'l':
                p->m.flags.longlong = 1;        /* Single l detected */
                ++fmt;
                if (*fmt == 'l') {              /* Does it follow by another l? */
                    p->m.flags.longlong = 2;    /* Second l detected */
                    ++fmt;
                }
                break;
            case 'L': break;
            case 'z': break;
            case 'j': break;
            case 't': break;
            default: detected = 0;
        }

        /* Check type */
        switch (*fmt) {
            case 'c':
                p->out_fn(p, va_arg(arg, char));
                break;
            case 'd':
            case 'i': {
                /* Check for different length parameters */
                p->m.base = 10;
                if (p->m.flags.longlong == 0) {
                    prv_signed_int_to_str(p, (signed int)va_arg(arg, signed int));
                } else if (p->m.flags.longlong == 1) {
                    prv_signed_long_int_to_str(p, (signed long int)va_arg(arg, signed long int));
#if LWPRINTF_CFG_SUPPORT_LONG_LONG
                } else if (p->m.flags.longlong == 2) {
                    prv_signed_longlong_int_to_str(p, (signed long long int)va_arg(arg, signed long long int));
                }
#endif /* LWPRINTF_CFG_SUPPORT_LONG_LONG */
                break;
            }
            case 'b':
            case 'B':
            case 'o':
            case 'u':
            case 'x':
            case 'X':
                if (*fmt == 'b' || *fmt == 'B') {
                    p->m.base = 2;
                } else if (*fmt == 'o') {
                    p->m.base = 8;
                } else if (*fmt == 'u') {
                    p->m.base = 10;
                } else if (*fmt == 'x' || *fmt == 'X') {
                    p->m.base = 16;
                    p->m.flags.uc = *fmt == 'X';/* Select if uppercase text shall be printed */
                }

                /* Check for different length parameters */
                if (p->m.flags.longlong == 0 || p->m.base == 2) {
                    unsigned int v;
                    switch (p->m.flags.char_short) {
                        case 2:     v = (unsigned int)((unsigned char)va_arg(arg, unsigned int)); break;
                        case 1:     v = (unsigned int)((unsigned short int)va_arg(arg, unsigned int)); break;
                        default:    v = (unsigned int)((unsigned int)va_arg(arg, unsigned int)); break;
                    }
                    prv_unsigned_int_to_str(p, v);
                } else if (p->m.flags.longlong == 1) {
                    prv_unsigned_long_int_to_str(p, (unsigned long int)va_arg(arg, unsigned long int));
                }
#if LWPRINTF_CFG_SUPPORT_LONG_LONG
                else if (p->m.flags.longlong == 2) {
                    prv_unsigned_longlong_int_to_str(p, (unsigned long long int)va_arg(arg, unsigned long long int));
                }
#endif /* LWPRINTF_CFG_SUPPORT_LONG_LONG */
                break;
            case 's': {
                const char* b = va_arg(arg, const char *);
                size_t len = strlen(b);

                /* Precision gives maximum output len */
                if (p->m.flags.precision) {
                    if (len > p->m.precision) {
                        len = p->m.precision;
                    }
                }
                prv_out_str(p, b, len);
                break;
            }
#if LWPRINTF_CFG_SUPPORT_TYPE_POINTER
            case 'p': {
                p->m.base = 16;                 /* Go to hex format */
                p->m.flags.uc = 1;              /* Uppercase numbers */
                p->m.flags.zero = 1;            /* Zero padding */
                p->m.width = sizeof(void *) * 2;/* Number is in hex format and byte is represented with 2 letters */

#if LWPRINTF_CFG_SUPPORT_LONG_LONG
                if (sizeof(void *) == sizeof(unsigned long long int)) {
                    prv_unsigned_longlong_int_to_str(p, (unsigned long long int)((uintptr_t)va_arg(arg, void *)));
                } else
#endif /* LWPRINTF_CFG_SUPPORT_LONG_LONG */
                if (sizeof(void *) == sizeof(unsigned long int)) {
                    prv_unsigned_long_int_to_str(p, (unsigned long int)((uintptr_t)va_arg(arg, void *)));
                } else {
                    prv_unsigned_int_to_str(p, (unsigned int)((uintptr_t)va_arg(arg, void *)));
                }
                break;
            }
#endif /* LWPRINTF_CFG_SUPPORT_TYPE_POINTER */
            case 'f':
            case 'F':
                prv_double_to_str(p, (double)va_arg(arg, double));
                break;
            case 'e':
            case 'E':
            case 'g':
            case 'G':
                break;
            case '%':
                p->out_fn(p, '%');
                break;
            default:
                p->out_fn(p, *fmt);
        }
        ++fmt;
    }
    return 1;
}

/**
 * \brief           Initialize LwPRINTF instance
 * \param[in,out]   lw: LwPRINTF working instance
 * \param[in]       out_fn: Output function used for print operation
 * \return          `1` on success, `0` otherwise
 */
uint8_t
lwprintf_init_ex(lwprintf_t* lw, lwprintf_output_fn out_fn) {
    LWPRINTF_GET_LW(lw)->out = out_fn;
    return 1;
}

/**
 * \brief           Print formatted data from variable argument list to the output
 * \param[in,out]   lw: LwPRINTF instance. Set to `NULL` to use default instance
 * \param[in]       format: C string that contains the text to be written to output
 * \param[in]       arg: A value identifying a variable arguments list initialized with `va_start`.
 *                      `va_list` is a special type defined in `<cstdarg>`.
 * \return          The number of characters that would have been written if `n` had been sufficiently large,
 *                      not counting the terminating null character.
 */
int
lwprintf_vprintf_ex(lwprintf_t* const lw, const char* format, va_list arg) {
    lwprintf_int_t f = {
        .lw = LWPRINTF_GET_LW(lw),
        .out_fn = prv_out_fn_print,
        .fmt = arg,
        .buff = NULL,
        .buff_size = 0
    };
    prv_format(&f, arg);
    return f.n;
}

/**
 * \brief           Print formatted data to the output
 * \param[in,out]   lw: LwPRINTF instance. Set to `NULL` to use default instance
 * \param[in]       format: C string that contains the text to be written to output
 *                  
 *                  ...: Optional arguments for format string
 * \return          The number of characters that would have been written if `n` had been sufficiently large,
 *                      not counting the terminating null character.
 */
int
lwprintf_printf_ex(lwprintf_t* const lw, const char* format, ...) {
    va_list va;
    int n;

    va_start(va, format);
    n = lwprintf_vprintf_ex(lw, format, va);
    va_end(va);

    return n;
}

/**
 * \brief           Write formatted data from variable argument list to sized buffer
 * \param[in,out]   lw: LwPRINTF instance. Set to `NULL` to use default instance
 * \param[in]       s: Pointer to a buffer where the resulting C-string is stored.
 *                      The buffer should have a size of at least `n` characters
 * \param[in]       n: Maximum number of bytes to be used in the buffer.
 *                      The generated string has a length of at most `n - 1`,
 *                      leaving space for the additional terminating null character
 * \param[in]       format: C string that contains a format string that follows the same specifications as format in printf
 * \param[in]       arg: A value identifying a variable arguments list initialized with `va_start`.
 *                      `va_list` is a special type defined in `<cstdarg>`.
 * \return          The number of characters that would have been written if `n` had been sufficiently large,
 *                      not counting the terminating null character.
 */
int
lwprintf_vsnprintf_ex(lwprintf_t* const lw, char* s, size_t n, const char* format, va_list arg) {
    lwprintf_int_t f = {
        .lw = LWPRINTF_GET_LW(lw),
        .out_fn = prv_out_fn_write_buff,
        .fmt = format,
        .buff = s,
        .buff_size = n
    };
    prv_format(&f, arg);
    return f.n;
}

/**
 * \brief           Write formatted data from variable argument list to sized buffer
 * \param[in,out]   lw: LwPRINTF instance. Set to `NULL` to use default instance
 * \param[in]       s: Pointer to a buffer where the resulting C-string is stored.
 *                      The buffer should have a size of at least `n` characters
 * \param[in]       n: Maximum number of bytes to be used in the buffer.
 *                      The generated string has a length of at most `n - 1`,
 *                      leaving space for the additional terminating null character
 * \param[in]       format: C string that contains a format string that follows the same specifications as format in printf
 *           
 *                  ...: Optional arguments for format string
 * \return          The number of characters that would have been written if `n` had been sufficiently large,
 *                      not counting the terminating null character.
 */
int
lwprintf_snprintf_ex(lwprintf_t* const lw, char* s, size_t n, const char* format, ...) {
    va_list va;
    int len;

    va_start(va, format);
    len = lwprintf_vsnprintf_ex(lw, s, n, format, va);
    va_end(va);

    return len;
}
