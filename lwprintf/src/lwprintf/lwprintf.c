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
 * \brief           Private output function declaration
 */
typedef int (*prv_output_fn)(lwprintf_t* lw, char* const buff, const char c, const size_t id, size_t buff_size);

/**
 * \brief           List of flags part for format
 */
typedef union {
    struct {
        uint8_t left_align : 1;                 /*!< Minus for left alignment */
        uint8_t plus : 1;                       /*!< Prepend + for positive numbers on the output */
        uint8_t space : 1;                      /*!< Prepend spaces. Not used with plus modifier */
        uint8_t zero : 1;                       /*!< Zero pad flag detection, add zeros if number length is less than width modifier */
        uint8_t thousands:1;                    /*!< Thousands has grouping applied */
        uint8_t hash : 1;
        uint8_t precision : 1;                  /*!< Precision flag has been used */

        /* Length modified flags */
        uint8_t l : 1;                          /*!< Flag indicating long */
        uint8_t ll : 1;                         /*!< Flag indicatin long-long number */
    } f;
    uint16_t flags;
} prv_flags_t;

typedef struct {
    prv_flags_t flags;
    int precision;
    int width;
} prv_parser_t;

/**
 * \brief           Output function for printf
 * \param[in]       ...
 */
static int
prv_out_fn_print(lwprintf_t* lw, char* const buff, const char c, const size_t id, size_t buff_size) {
    lw->out(c, lw);                             /*!< Send character to output */

    return 1;
}

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

static uint8_t
prv_format(lwprintf_t* const lw, const prv_output_fn out_fn,
            const char* fmt, char* buff,
            const size_t buff_size, va_list vl) {
    prv_parser_t p;
    size_t idx = 0;
    uint8_t detected = 0;

    while (fmt != NULL && *fmt != '\0') {
        /* Parse format */
        /* %[flags][width][.precision][length]type */
        /* Go to https://en.wikipedia.org/wiki/Printf_format_string for more info */

        /* Detect beginning */
        if (*fmt != '%') {
            out_fn(lw, buff, *fmt, idx++, buff_size);   /* Output character */
            ++fmt;
            continue;
        }
        fmt++;

        /* Check [flags] */
        /* If can have multiple flags in any order */
        p.flags.flags = 0;
        do {
            detected = 1;
            switch (*fmt) {
                case '-':   p.flags.f.left_align = 1;   break;
                case '+':   p.flags.f.plus = 1;         break;
                case ' ':   p.flags.f.space = 1;        break;
                case '0':   p.flags.f.zero = 1;         break;
                case '\'':  p.flags.f.thousands = 1;    break;
                case '#':   p.flags.f.hash = 1;         break;
                default:    detected = 0;               break;
            }
            if (detected) {
                fmt++;
            }
        } while (detected);

        /* Check [width] */
        p.width = 0;
        if (CHARISNUM(*fmt)) {
            p.width = prv_parse_num(&fmt);
        }

        /* Check [.precision] */
        p.precision = 0;
        if (*fmt == '.') {
            p.flags.f.precision = 1;
            ++fmt;
            if (*fmt == '*') {
                const int pr = (int)va_arg(vl, int);
                p.precision = pr > 0 ? pr : 0;
            } else if (CHARISNUM(*fmt)) {
                p.precision = prv_parse_num(&fmt);
            }
        }

        /* Check [length] */
        detected = 1;
        switch (*fmt) {
            case 'l':
                p.flags.l = 1;
                ++fmt;
                if (*fmt == 'l') {
                    p.flags.ll = 1;
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
            case 'd':
            case 'D':
                out_fn(lw, buff, 'd', idx++, buff_size);
                break;
            default:
                out_fn(lw, buff, *fmt, idx++, buff_size);
        }
        ++fmt;
    }

    return idx;
}

uint8_t
lwprintf_init(lwprintf_t* lw, lwprintf_output_fn out_fn) {
    lw->out = out_fn;
    return 1;
}

int
lwprintf_printf(lwprintf_t* const lw, const char* fmt, ...) {
    va_list va;
    char buff[1];
    int n;

    va_start(va, fmt);
    n = prv_format(lw, prv_out_fn_print, fmt, buff, sizeof(buff), va);
    va_end(va);

    return n;
}
