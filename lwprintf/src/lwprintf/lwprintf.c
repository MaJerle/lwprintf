/**
 * \file            lwprintff.c
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
#include "lwprintf/lwprintf.h"
#include <limits.h>

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
 */
typedef int (*prv_output_fn)(struct lwprintf_int* lwi, const char c);

typedef struct lwprintf_int {
    lwprintf_t* lw;                             /*!< Instance */
    const char* fmt;                            /*!< Format strin */
    char* const buff;                           /*!< Pointer to buffer when not using print option */
    const size_t buff_size;                     /*!< Buffer size of input buffer (when used) */
    size_t n;                                   /*!< Full length of formatted text */
    prv_output_fn out_fn;                       /*!< Output internal function */
    char* buff_tmp;                             /*!< Pointer to temporary buffer for number conversion */

    /* This must all be reset every time new % is detected */
    struct {
        struct {
            uint8_t left_align : 1;             /*!< Minus for left alignment */
            uint8_t plus : 1;                   /*!< Prepend + for positive numbers on the output */
            uint8_t space : 1;                  /*!< Prepend spaces. Not used with plus modifier */
            uint8_t zero : 1;                   /*!< Zero pad flag detection, add zeros if number length is less than width modifier */
            uint8_t thousands : 1;              /*!< Thousands has grouping applied */
            uint8_t hash : 1;
            uint8_t precision : 1;              /*!< Precision flag has been used */

            /* Length modified flags */
            uint8_t longlong : 2;               /*!< Flag indicatin long-long number */
            uint8_t uc : 1;                     /*!< Uppercase flag */
            uint8_t is_negative : 1;            /*!< Status if number is negative */
        } flags;
        int precision;                          /*!< Selected precision */
        int width;                              /*!< Text width indicator */
        uint8_t base;                           /*!< Base for number format output */
    } m;
} lwprintf_int_t;

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
 * \param[in]       lwi: Internal working structure
 * \param[in]       c: Character to print
 * \return          `1` on success, `0` otherwise
 */
static int
prv_out_fn_print(lwprintf_int_t* lwi, const char c) {
    lwi->lw->out(c, lwi->lw);                   /*!< Send character to output */
    ++lwi->n;
    return 1;
}

/**
 * \brief           Output function to generate buffer data
 * \param[in]       lwi: Internal working structure
 * \param[in]       c: Character to write
 * \return          `1` on success, `0` otherwise
 */
static int
prv_out_fn_write_buff(lwprintf_int_t* lwi, const char c) {
    if (lwi->n < (lwi->buff_size - 1)) {
        lwi->buff[lwi->n++] = c;
        lwi->buff[lwi->n] = '\0';
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
 * \brief           Output generate string from numbers/digits
 * Paddings before and after are applied at this stage
 * \param[in]       p: Internal working structure
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

    /* Check for width */
    if (p->m.width > 0 && p->m.flags.is_negative) {
        --p->m.width;
    }
    
    /* Add negative sign before when zeros are used to fill width */
    if (p->m.flags.is_negative && p->m.flags.zero) {
        p->out_fn(p, '-');
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

    /* Actual value */
    for (uint8_t i = 0; i < buff_size; ++i) {
        p->out_fn(p, buff[i]);
    }
    /* Left alignment, but only with spaces */
    if (p->m.flags.left_align) {
        for (size_t i = buff_size; i < p->m.width; ++i) {
            p->out_fn(p, ' ');
        }
    }
    return 1;
}

static int
unsigned_int_to_str(lwprintf_int_t* p, unsigned int num) {
    unsigned int d;
    uint8_t digits_cnt;
    char c;

    /* Format string */
    for (digits_cnt = 0; num > 0; num /= p->m.base, ++digits_cnt) {
        d = num % p->m.base;
        c = d + (d >= 10 ? ((p->m.flags.uc ? 'A' : 'a') - 10) : '0');
        p->buff_tmp[digits_cnt] = c;
    }
    p->buff_tmp[digits_cnt] = 0;
    prv_rotate_string(p->buff_tmp, digits_cnt); /* Rotate string as it was written in opposite direction */
    prv_out_str(p, p->buff_tmp, digits_cnt);    /* Output generated string */

    return 1;
}

static int
unsigned_long_int_to_str(lwprintf_int_t* p, unsigned long int num) {
    unsigned long int d;
    uint8_t digits_cnt;
    char c;

    /* Format string */
    for (digits_cnt = 0; num > 0; num /= p->m.base, ++digits_cnt) {
        d = num % p->m.base;
        c = d + (d >= 10 ? ((p->m.flags.uc ? 'A' : 'a') - 10) : '0');
        p->buff_tmp[digits_cnt] = c;
    }
    p->buff_tmp[digits_cnt] = 0;
    prv_rotate_string(p->buff_tmp, digits_cnt); /* Rotate string as it was written in opposite direction */
    prv_out_str(p, p->buff_tmp, digits_cnt);    /* Output generated string */

    return 1;
}

static int
unsigned_longlong_int_to_str(lwprintf_int_t* p, unsigned long long int num) {
    unsigned long long int d;
    uint8_t digits_cnt;
    char c;

    /* Format string */
    for (digits_cnt = 0; num > 0; num /= p->m.base, ++digits_cnt) {
        d = num % p->m.base;
        c = d + (d >= 10 ? ((p->m.flags.uc ? 'A' : 'a') - 10) : '0');
        p->buff_tmp[digits_cnt] = c;
    }
    p->buff_tmp[digits_cnt] = 0;
    prv_rotate_string(p->buff_tmp, digits_cnt); /* Rotate string as it was written in opposite direction */
    prv_out_str(p, p->buff_tmp, digits_cnt);    /* Output generated string */

    return 1;
}

static int
signed_int_to_str(lwprintf_int_t* p, signed int num) {
    if (num < 0) {
        p->m.flags.is_negative = 1;
        num = -num;
    }
    return unsigned_int_to_str(p, num);
}

static int
signed_long_int_to_str(lwprintf_int_t* p, signed long int num) {
    if (num < 0) {
        p->m.flags.is_negative = 1;
        num = -num;
    }
    return unsigned_long_int_to_str(p, num);
}

static int
signed_longlong_int_to_str(lwprintf_int_t* p, signed long long int num) {
    if (num < 0) {
        p->m.flags.is_negative = 1;
        num = -num;
    }
    return unsigned_longlong_int_to_str(p, num);
}

static uint8_t
prv_format(lwprintf_int_t* p, va_list vl) {
    uint8_t detected = 0;
    char buff_tmp[23];
    const char* fmt = p->fmt;

    p->buff_tmp = buff_tmp;
    while (fmt != NULL && *fmt != '\0') {
        /* Parse format */
        /* %[flags][width][.precision][length]type */
        /* Go to https://en.wikipedia.org/wiki/Printf_format_string for more info */
        memset(&p->m, 0x00, sizeof(p->m));      /* Reset structure */

        /* Detect beginning */
        if (*fmt != '%') {
            p->out_fn(p, *fmt);                 /* Output character */
            ++fmt;
            continue;
        }
        ++fmt;

        /* Check [flags] */
        /* It can have multiple flags in any order */
        do {
            detected = 1;
            switch (*fmt) {
                case '-':   p->m.flags.left_align = 1;  break;
                case '+':   p->m.flags.plus = 1;        break;
                case ' ':   p->m.flags.space = 1;       break;
                case '0':   p->m.flags.zero = 1;        break;
                case '\'':  p->m.flags.thousands = 1;   break;
                case '#':   p->m.flags.hash = 1;        break;
                default:    detected = 0;               break;
            }
            if (detected) {
                fmt++;
            }
        } while (detected);

        /* Check [width] */
        p->m.width = 0;
        if (CHARISNUM(*fmt)) {
            p->m.width = prv_parse_num(&fmt);
        }

        /* Check [.precision] */
        p->m.precision = 0;
        if (*fmt == '.') {
            p->m.flags.precision = 1;
            ++fmt;
            if (*fmt == '*') {
                const int pr = (int)va_arg(vl, int);
                p->m.precision = pr > 0 ? pr : 0;
            } else if (CHARISNUM(*fmt)) {
                p->m.precision = prv_parse_num(&fmt);
            }
        }

        /* Check [length] */
        detected = 1;
        switch (*fmt) {
            case 'l':
                p->m.flags.longlong = 1;
                ++fmt;
                if (*fmt == 'l') {
                    p->m.flags.longlong = 2;
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
                p->out_fn(p, va_arg(vl, char));
                break;
            case 'd':
            case 'i': {
                /* Check for different length parameters */
                p->m.base = 10;
                if (p->m.flags.longlong == 0) {
                    signed_int_to_str(p, (signed int)va_arg(vl, signed int));
                } else if (p->m.flags.longlong == 2) {
                    signed_longlong_int_to_str(p, (signed long long int)va_arg(vl, signed long long int));
                } else if (p->m.flags.longlong == 1) {
                    signed_long_int_to_str(p, (signed long int)va_arg(vl, signed long int));
                }
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

                /* TODO: base 2 will overflow when more than buff size bits are used */
                
                /* Check for different length parameters */
                if (p->m.flags.longlong == 0 || p->m.base == 2) {
                    unsigned_int_to_str(p, (unsigned int)va_arg(vl, unsigned int));
                } else if (p->m.flags.longlong == 2) {
                    unsigned_longlong_int_to_str(p, (unsigned long long int)va_arg(vl, unsigned long long int));
                } else if (p->m.flags.longlong == 1) {
                    unsigned_long_int_to_str(p, (unsigned long int)va_arg(vl, unsigned long int));
                }
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
lwprintf_init(lwprintf_t* lw, lwprintf_output_fn out_fn) {
    lw->out = out_fn;
    return 1;
}

/**
 * \brief           Print data to the output directly
 * \param[in,out]   lw: LwPRINTF working instance.
 *                      Set to `NULL` to use default one
 * \param[in]       fmt: Format string
 * \param[in]       ... Optional arguments for format
 * \return          `1` on success, `0` otherwise
 */
int
lwprintf_printf(lwprintf_t* const lw, const char* fmt, ...) {
    va_list va;
    lwprintf_int_t format = {
        .lw = lw,
        .out_fn = prv_out_fn_print,
        .fmt = fmt,
        .buff = NULL,
        .buff_size = 0
    };

    va_start(va, fmt);
    prv_format(&format, va);
    va_end(va);

    return format.n;
}

int
lwprintf_snprintf(lwprintf_t* const lw, char* buff, size_t buff_size, const char* fmt, ...) {
    va_list va;
    lwprintf_int_t format = {
        .lw = lw,
        .out_fn = prv_out_fn_write_buff,
        .fmt = fmt,
        .buff = buff,
        .buff_size = buff_size
    };

    va_start(va, fmt);
    prv_format(&format, va);
    va_end(va);

    return format.n;
}
