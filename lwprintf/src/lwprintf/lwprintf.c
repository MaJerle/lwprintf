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
        } flags;
        int precision;                          /*!< Selected precision */
        int width;                              /*!< Text width indicator */
        uint8_t base;                           /*!< Base for number format output */
    } m;
} lwprintf_int_t;

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

static int
unsigned_int_to_str(lwprintf_int_t* p, unsigned int num) {
    char buff[12];
    unsigned int d;
    uint8_t digits_cnt;
    char c;

    /* Format string */
    for (digits_cnt = 0; num > 0; num /= p->m.base, ++digits_cnt) {
        d = num % p->m.base;
        c = d + (d > 10 ? ((p->m.flags.uc ? 'A' : 'a') - 10) : '0');
        buff[digits_cnt] = c;
    }
    
    /* Rotate string */
    for (uint8_t i = 0; i < digits_cnt / 2; ++i) {
        char t = buff[i];
        buff[i] = buff[digits_cnt - i - 1];
        buff[digits_cnt - i - 1] = t;
    }

    /* Output string */
    for (uint8_t i = 0; i < digits_cnt; ++i) {
        p->out_fn(p, buff[i]);
    }
    return digits_cnt;
}

static int
unsigned_long_int_to_str(lwprintf_int_t* p, unsigned long int num) {

}

static int
unsigned_longlong_int_to_str(lwprintf_int_t* p, unsigned long long int num) {

}

static uint8_t
prv_format(lwprintf_int_t* p, va_list vl) {
    uint8_t detected = 0;
    const char* fmt = p->fmt;

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
                p->m.base = 10;
                int num = va_arg(vl, int);
                p->out_fn(p, 'd');
                break;
            }
            case 'b':
            case 'B':
                p->m.base = 2;
                unsigned_int_to_str(p, va_arg(vl, unsigned int));
                break;
            case 'o':
                p->m.base = 8;
                unsigned_int_to_str(p, va_arg(vl, unsigned int));
                break;
            case 'u':
                p->m.base = 10;
                unsigned_int_to_str(p, va_arg(vl, unsigned int));
                break;
            case 'x':
            case 'X':
                p->m.base = 16;
                p->m.flags.uc = *fmt == 'X';    /* Select if uppercase text shall be printed */
                unsigned_int_to_str(p, va_arg(vl, unsigned int));
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
