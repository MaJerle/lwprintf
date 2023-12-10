/**
 * \file            lwprintf.c
 * \brief           Lightweight stdio manager
 */

/*
 * Copyright (c) 2023 Tilen MAJERLE
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
 * Version:         v1.0.5
 */
#include "lwprintf/lwprintf.h"
#include <float.h>
#include <limits.h>
#include <stdint.h>

#if LWPRINTF_CFG_OS
#include "system/lwprintf_sys.h"
#endif /* LWPRINTF_CFG_OS */

/* Static checks */
#if LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING && !LWPRINTF_CFG_SUPPORT_TYPE_FLOAT
#error "Cannot use engineering type without float!"
#endif /* LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING && !LWPRINTF_CFG_SUPPORT_TYPE_FLOAT */
#if !LWPRINTF_CFG_OS && LWPRINTF_CFG_OS_MANUAL_PROTECT
#error "LWPRINTF_CFG_OS_MANUAL_PROTECT can only be used if LWPRINTF_CFG_OS is enabled"
#endif /* !LWPRINTF_CFG_OS && LWPRINTF_CFG_OS_MANUAL_PROTECT */

#define CHARISNUM(x)     ((x) >= '0' && (x) <= '9')
#define CHARTONUM(x)     ((x) - '0')
#define IS_PRINT_MODE(p) ((p)->out_fn == prv_out_fn_print)

/* Define custom types */
#if LWPRINTF_CFG_SUPPORT_LONG_LONG
typedef long long int float_long_t;
#else
typedef long int float_long_t;
#endif /* LWPRINTF_CFG_SUPPORT_LONG_LONG */

/**
 * \brief           Float number splitted by parts
 */
typedef struct {
    float_long_t integer_part; /*!< Integer type of double number */
    double decimal_part_dbl;   /*!< Decimal part of double number multiplied by 10^precision */
    float_long_t decimal_part; /*!< Decimal part of double number in integer format */
    double diff;               /*!< Difference between decimal parts (double - int) */

    short digits_cnt_integer_part;        /*!< Number of digits for integer part */
    short digits_cnt_decimal_part;        /*!< Number of digits for decimal part */
    short digits_cnt_decimal_part_useful; /*!< Number of useful digits to print */
} float_num_t;

#if LWPRINTF_CFG_SUPPORT_TYPE_FLOAT
/* Powers of 10 from beginning up to precision level */
static const float_long_t powers_of_10[] = {
    (float_long_t)1E00, (float_long_t)1E01, (float_long_t)1E02, (float_long_t)1E03, (float_long_t)1E04,
    (float_long_t)1E05, (float_long_t)1E06, (float_long_t)1E07, (float_long_t)1E08, (float_long_t)1E09,
#if LWPRINTF_CFG_SUPPORT_LONG_LONG
    (float_long_t)1E10, (float_long_t)1E11, (float_long_t)1E12, (float_long_t)1E13, (float_long_t)1E14,
    (float_long_t)1E15, (float_long_t)1E16, (float_long_t)1E17, (float_long_t)1E18,
#endif /* LWPRINTF_CFG_SUPPORT_LONG_LONG */
};
#endif /* LWPRINTF_CFG_SUPPORT_TYPE_FLOAT */
#define FLOAT_MAX_B_ENG (powers_of_10[LWPRINTF_ARRAYSIZE(powers_of_10) - 1])

/**
 * \brief           Outputs any integer type to stream
 * Implemented as big macro since `d`, `digit` and `num` are of different types vs int size
 */
#define OUTPUT_ANY_INT_TYPE(ttype, num)                                                                                \
    {                                                                                                                  \
        ttype den, digit;                                                                                              \
        uint8_t digits_cnt;                                                                                            \
        char chr;                                                                                                      \
                                                                                                                       \
        /* Check if number is zero */                                                                                  \
        lwi->m.flags.is_num_zero = (num) == 0;                                                                         \
        if ((num) == 0) {                                                                                              \
            prv_out_str_before(lwi, 1);                                                                                \
            lwi->out_fn(lwi, '0');                                                                                     \
            prv_out_str_after(lwi, 1);                                                                                 \
        } else {                                                                                                       \
            /* Start with digits length */                                                                             \
            for (digits_cnt = 0, den = (num); den > 0; ++digits_cnt, den /= lwi->m.base) {}                            \
            for (den = 1; ((num) / den) >= lwi->m.base; den *= lwi->m.base) {}                                         \
                                                                                                                       \
            prv_out_str_before(lwi, digits_cnt);                                                                       \
            for (; den > 0;) {                                                                                         \
                digit = (num) / den;                                                                                   \
                (num) = (num) % den;                                                                                   \
                den = den / lwi->m.base;                                                                               \
                chr = (char)digit + (char)(digit >= 10 ? ((lwi->m.flags.uc ? 'A' : 'a') - 10) : '0');                  \
                lwi->out_fn(lwi, chr);                                                                                 \
            }                                                                                                          \
            prv_out_str_after(lwi, digits_cnt);                                                                        \
        }                                                                                                              \
    }

/**
 * \brief           Check for negative input number before outputting signed integers
 * \param[in]       pp: Parsing object
 * \param[in]       nnum: Number to check
 */
#define SIGNED_CHECK_NEGATIVE(pp, nnum)                                                                                \
    {                                                                                                                  \
        if ((nnum) < 0) {                                                                                              \
            (pp)->m.flags.is_negative = 1;                                                                             \
            (nnum) = -(nnum);                                                                                          \
        }                                                                                                              \
    }

/**
 * \brief           Forward declaration
 */
struct lwprintf_int;

/**
 * \brief           Private output function declaration
 * \param[in]       lwi: Internal working structure
 * \param[in]       chr: Character to print
 */
typedef int (*prv_output_fn)(struct lwprintf_int* lwi, const char chr);

/**
 * \brief           Internal structure
 */
typedef struct lwprintf_int {
    lwprintf_t* lwobj;          /*!< Instance handle */
    const char* fmt;            /*!< Format string */
    char* const buff;           /*!< Pointer to buffer when not using print option */
    const size_t buff_size;     /*!< Buffer size of input buffer (when used) */
    size_t n_len;               /*!< Full length of formatted text */
    prv_output_fn out_fn;       /*!< Output internal function */
    uint8_t is_print_cancelled; /*!< Status if print should be cancelled */

    /* This must all be reset every time new % is detected */
    struct {
        struct {
            uint8_t left_align : 1; /*!< Minus for left alignment */
            uint8_t plus       : 1; /*!< Prepend + for positive numbers on the output */
            uint8_t space      : 1; /*!< Prepend spaces. Not used with plus modifier */
            uint8_t zero : 1; /*!< Zero pad flag detection, add zeros if number length is less than width modifier */
            uint8_t thousands   : 1; /*!< Thousands has grouping applied */
            uint8_t alt         : 1; /*!< Alternate form with hash */
            uint8_t precision   : 1; /*!< Precision flag has been used */

            /* Length modified flags */
            uint8_t longlong    : 2; /*!< Flag indicatin long-long number, used with 'l' (1) or 'll' (2) mode */
            uint8_t char_short  : 2; /*!< Used for 'h' (1 = short) or 'hh' (2 = char) length modifier */
            uint8_t sz_t        : 1; /*!< Status for size_t length integer type */
            uint8_t umax_t      : 1; /*!< Status for uintmax_z length integer type */

            uint8_t uc          : 1; /*!< Uppercase flag */
            uint8_t is_negative : 1; /*!< Status if number is negative */
            uint8_t is_num_zero : 1; /*!< Status if input number is zero */
        } flags;                     /*!< List of flags */

        int precision; /*!< Selected precision */
        int width;     /*!< Text width indicator */
        uint8_t base;  /*!< Base for number format output */
        char type;     /*!< Format type */
    } m;               /*!< Block that is reset on every start of format */
} lwprintf_int_t;

/**
 * \brief           Get LwPRINTF instance based on user input
 * \param[in]       lwi: LwPRINTF instance.
 *                      Set to `NULL` for default instance
 */
#define LWPRINTF_GET_LWOBJ(ptr) ((ptr) != NULL ? (ptr) : (&lwprintf_default))

/**
 * \brief           LwPRINTF default structure used by application
 */
static lwprintf_t lwprintf_default;

/**
 * \brief           Output function to print data
 * \param[in]       ptr: LwPRINTF internal instance
 * \param[in]       chr: Character to print
 * \return          `1` on success, `0` otherwise
 */
static int
prv_out_fn_print(lwprintf_int_t* lwi, const char chr) {
    if (lwi->is_print_cancelled) {
        return 0;
    }
    /*!< Send character to output */
    if (!lwi->lwobj->out_fn(chr, lwi->lwobj)) {
        lwi->is_print_cancelled = 1;
    }
    if (chr != '\0' && !lwi->is_print_cancelled) {
        ++lwi->n_len;
    }
    return 1;
}

/**
 * \brief           Output function to generate buffer data
 * \param[in]       lwi: LwPRINTF internal instance
 * \param[in]       chr: Character to write
 * \return          `1` on success, `0` otherwise
 */
static int
prv_out_fn_write_buff(lwprintf_int_t* lwi, const char chr) {
    if (lwi->n_len < (lwi->buff_size - 1)) {
        lwi->buff[lwi->n_len] = chr;
        if (chr != '\0') {
            lwi->buff[++lwi->n_len] = '\0';
        }
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
    int num = 0;

    for (; CHARISNUM(*fmt); ++fmt) {
        num = (int)10 * num + CHARTONUM(*fmt);
    }
    *format = fmt;
    return num;
}

/**
 * \brief           Format data that are printed before actual value
 * \param[in,out]   lwi: LwPRINTF internal instance
 * \param[in]       buff_size: Expected buffer size of output string
 * \return          `1` on success, `0` otherwise
 */
static int
prv_out_str_before(lwprintf_int_t* lwi, size_t buff_size) {
    /* Check for width */
    if (lwi->m.width > 0
        /* If number is negative, add negative sign or if positive and has plus sign forced */
        && (lwi->m.flags.is_negative || lwi->m.flags.plus)) {
        --lwi->m.width;
    }

    /* Check for alternate mode */
    if (lwi->m.flags.alt && !lwi->m.flags.is_num_zero) {
        if (lwi->m.base == 8) {
            if (lwi->m.width > 0) {
                --lwi->m.width;
            }
        } else if (lwi->m.base == 16 || lwi->m.base == 2) {
            if (lwi->m.width >= 2) {
                lwi->m.width -= 2;
            } else {
                lwi->m.width = 0;
            }
        }
    }

    /* Add negative sign (or positive in case of + flag or space in case of space flag) before when zeros are used to fill width */
    if (lwi->m.flags.zero) {
        if (lwi->m.flags.is_negative) {
            lwi->out_fn(lwi, '-');
        } else if (lwi->m.flags.plus) {
            lwi->out_fn(lwi, '+');
        } else if (lwi->m.flags.space) {
            lwi->out_fn(lwi, ' ');
        }
    }

    /* Check for flags output */
    if (lwi->m.flags.alt && !lwi->m.flags.is_num_zero) {
        if (lwi->m.base == 8) {
            lwi->out_fn(lwi, '0');
        } else if (lwi->m.base == 16) {
            lwi->out_fn(lwi, '0');
            lwi->out_fn(lwi, lwi->m.flags.uc ? 'X' : 'x');
        } else if (lwi->m.base == 2) {
            lwi->out_fn(lwi, '0');
            lwi->out_fn(lwi, lwi->m.flags.uc ? 'B' : 'b');
        }
    }

    /* Right alignment, spaces or zeros */
    if (!lwi->m.flags.left_align && lwi->m.width > 0) {
        for (size_t idx = buff_size; !lwi->m.flags.left_align && idx < (size_t)lwi->m.width; ++idx) {
            lwi->out_fn(lwi, lwi->m.flags.zero ? '0' : ' ');
        }
    }

    /* Add negative sign here when spaces are used for width */
    if (!lwi->m.flags.zero) {
        if (lwi->m.flags.is_negative) {
            lwi->out_fn(lwi, '-');
        } else if (lwi->m.flags.plus) {
            lwi->out_fn(lwi, '+');
        } else if (lwi->m.flags.space && buff_size >= (size_t)lwi->m.width) {
            lwi->out_fn(lwi, ' ');
        }
    }

    return 1;
}

/**
 * \brief           Format data that are printed after actual value
 * \param[in,out]   lwi: LwPRINTF internal instance
 * \param[in]       buff_size: Expected buffer size of output string
 * \return          `1` on success, `0` otherwise
 */
static int
prv_out_str_after(lwprintf_int_t* lwi, size_t buff_size) {
    /* Left alignment, but only with spaces */
    if (lwi->m.flags.left_align) {
        for (size_t idx = buff_size; idx < (size_t)lwi->m.width; ++idx) {
            lwi->out_fn(lwi, ' ');
        }
    }
    return 1;
}

/**
 * \brief           Output raw string without any formatting
 * \param[in,out]   lwi: LwPRINTF internal instance
 * \param[in]       buff: Buffer string
 * \param[in]       buff_size: Length of buffer to output
 * \return          `1` on success, `0` otherwise
 */
static int
prv_out_str_raw(lwprintf_int_t* lwi, const char* buff, size_t buff_size) {
    for (size_t idx = 0; idx < buff_size; ++idx) {
        lwi->out_fn(lwi, buff[idx]);
    }
    return 1;
}

/**
 * \brief           Output generated string from numbers/digits
 * Paddings before and after are applied at this stage
 *
 * \param[in,out]   lwi: LwPRINTF internal instance
 * \param[in]       buff: Buffer string
 * \param[in]       buff_size: Length of buffer to output
 * \return          `1` on success, `0` otherwise
 */
static int
prv_out_str(lwprintf_int_t* lwi, const char* buff, size_t buff_size) {
    prv_out_str_before(lwi, buff_size);    /* Implement pre-format */
    prv_out_str_raw(lwi, buff, buff_size); /* Print actual string */
    prv_out_str_after(lwi, buff_size);     /* Implement post-format */

    return 1;
}

/**
 * \brief           Convert `unsigned int` to string
 * \param[in,out]   lwi: LwPRINTF internal instance
 * \param[in]       num: Number to convert to string
 * \return          `1` on success, `0` otherwise
 */
static int
prv_unsigned_int_to_str(lwprintf_int_t* lwi, unsigned int num) {
    OUTPUT_ANY_INT_TYPE(unsigned int, num);
    return 1;
}

/**
 * \brief           Convert `unsigned long` to string
 * \param[in,out]   lwi: LwPRINTF internal instance
 * \param[in]       num: Number to convert to string
 * \return          `1` on success, `0` otherwise
 */
static int
prv_unsigned_long_int_to_str(lwprintf_int_t* lwi, unsigned long int num) {
    OUTPUT_ANY_INT_TYPE(unsigned long int, num);
    return 1;
}

#if LWPRINTF_CFG_SUPPORT_LONG_LONG

/**
 * \brief           Convert `unsigned long-long` to string
 * \param[in,out]   lwi: LwPRINTF internal instance
 * \param[in]       num: Number to convert to string
 * \return          `1` on success, `0` otherwise
 */
static int
prv_unsigned_longlong_int_to_str(lwprintf_int_t* lwi, unsigned long long int num) {
    OUTPUT_ANY_INT_TYPE(unsigned long long int, num);
    return 1;
}

#endif /* LWPRINTF_CFG_SUPPORT_LONG_LONG */

#if LWPRINTF_CFG_SUPPORT_TYPE_POINTER

/**
 * \brief           Convert `uintptr_t` to string
 * \param[in,out]   lwi: LwPRINTF internal instance
 * \param[in]       num: Number to convert to string
 * \return          `1` on success, `0` otherwise
 */
static int
prv_uintptr_to_str(lwprintf_int_t* lwi, uintptr_t num) {
    OUTPUT_ANY_INT_TYPE(uintptr_t, num);
    return 1;
}

#endif /* LWPRINTF_CFG_SUPPORT_TYPE_POINTER */

/**
 * \brief           Convert `size_t` number to string
 * \param[in,out]   lwi: LwPRINTF internal instance
 * \param[in]       num: Number to convert to string
 * \return          `1` on success, `0` otherwise
 */
static int
prv_sizet_to_str(lwprintf_int_t* lwi, size_t num) {
    OUTPUT_ANY_INT_TYPE(size_t, num);
    return 1;
}

/**
 * \brief           Convert `uintmax_t` number to string
 * \param[in,out]   lwi: LwPRINTF internal instance
 * \param[in]       num: Number to convert to string
 * \return          `1` on success, `0` otherwise
 */
static int
prv_umaxt_to_str(lwprintf_int_t* lwi, uintmax_t num) {
    OUTPUT_ANY_INT_TYPE(uintmax_t, num);
    return 1;
}

/**
 * \brief           Convert signed int to string
 * \param[in,out]   lwi: LwPRINTF internal instance
 * \param[in]       num: Number to convert to string
 * \return          `1` on success, `0` otherwise
 */
static int
prv_signed_int_to_str(lwprintf_int_t* lwi, signed int num) {
    SIGNED_CHECK_NEGATIVE(lwi, num);
    return prv_unsigned_int_to_str(lwi, num);
}

/**
 * \brief           Convert signed long int to string
 * \param[in,out]   lwi: LwPRINTF instance
 * \param[in]       num: Number to convert to string
 * \return          `1` on success, `0` otherwise
 */
static int
prv_signed_long_int_to_str(lwprintf_int_t* lwi, signed long int num) {
    SIGNED_CHECK_NEGATIVE(lwi, num);
    return prv_unsigned_long_int_to_str(lwi, num);
}

#if LWPRINTF_CFG_SUPPORT_LONG_LONG

/**
 * \brief           Convert signed long-long int to string
 * \param[in,out]   lwi: LwPRINTF internal instance
 * \param[in]       num: Number to convert to string
 * \return          `1` on success, `0` otherwise
 */
static int
prv_signed_longlong_int_to_str(lwprintf_int_t* lwi, signed long long int num) {
    SIGNED_CHECK_NEGATIVE(lwi, num);
    return prv_unsigned_longlong_int_to_str(lwi, num);
}

#endif /* LWPRINTF_CFG_SUPPORT_LONG_LONG */

#if LWPRINTF_CFG_SUPPORT_TYPE_FLOAT

/**
 * \brief           Calculate necessary parameters for input number
 * \param[in,out]   lwi: LwPRINTF internal instance
 * \param[in]       n: Float number instance
 * \param[in]       num: Input number
 * \param[in]       type: Format type
 */
static void
prv_calculate_dbl_num_data(lwprintf_int_t* lwi, float_num_t* n, double num, const char type) {
    memset(n, 0x00, sizeof(*n));

    if (lwi->m.precision >= (int)LWPRINTF_ARRAYSIZE(powers_of_10)) {
        lwi->m.precision = (int)LWPRINTF_ARRAYSIZE(powers_of_10) - 1;
    }

    /*
     * Get integer and decimal parts, both in integer formats
     *
     * As an example, with input number of 12.345678 and precision digits set as 4, then result is the following:
     *
     * integer_part = 12            -> Actual integer part of the double number
     * decimal_part_dbl = 3456.78   -> Decimal part multiplied by 10^precision, keeping it in double format
     * decimal_part = 3456          -> Integer part of decimal number
     * diff = 0.78                  -> Difference between actual decimal and integer part of decimal
     *                                  This is used for rounding of last digit (if necessary)
     */
    num += 0.000000000000005;
    n->integer_part = (float_long_t)num;
    n->decimal_part_dbl = (num - (double)n->integer_part) * (double)powers_of_10[lwi->m.precision];
    n->decimal_part = (float_long_t)n->decimal_part_dbl;
    n->diff = n->decimal_part_dbl - (double)((float_long_t)n->decimal_part);

    /* Rounding check of last digit */
    if (n->diff > 0.5f) {
        ++n->decimal_part;
        if (n->decimal_part >= powers_of_10[lwi->m.precision]) {
            n->decimal_part = 0;
            ++n->integer_part;
        }
    } else if (n->diff < 0.5f) {
        /* Used in separate if, since comparing float to == will certainly result to false */
    } else {
        /* Difference is exactly 0.5 */
        if (n->decimal_part == 0) {
            ++n->integer_part;
        } else {
            ++n->decimal_part;
        }
    }

    /* Calculate number of digits for integer and decimal parts */
    if (n->integer_part == 0) {
        n->digits_cnt_integer_part = 1;
    } else {
        float_long_t tmp;
        for (n->digits_cnt_integer_part = 0, tmp = n->integer_part; tmp > 0; ++n->digits_cnt_integer_part, tmp /= 10) {}
    }
    n->digits_cnt_decimal_part = (short)lwi->m.precision;

#if LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING
    /* Calculate minimum useful digits for decimal (excl last useless zeros) */
    if (type == 'g') {
        float_long_t tmp = n->decimal_part;
        short adder, i;

        for (adder = 0, i = 0; tmp > 0 || i < (short)lwi->m.precision;
             tmp /= 10, n->digits_cnt_decimal_part_useful += adder, ++i) {
            if (adder == 0 && (tmp % 10) > 0) {
                adder = 1;
            }
        }
    } else
#endif /* LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING */
    {
        n->digits_cnt_decimal_part_useful = (short)lwi->m.precision;
    }
}

/**
 * \brief           Convert double number to string
 * \param[in,out]   lwi: LwPRINTF internal instance
 * \param[in]       num: Number to convert to string
 * \return          `1` on success, `0` otherwise
 */
static int
prv_double_to_str(lwprintf_int_t* lwi, double in_num) {
    float_num_t dblnum;
    double orig_num = in_num;
    int digits_cnt, chosen_precision, i;
#if LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING
    int exp_cnt = 0;
#endif /* LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING */
    char def_type = lwi->m.type;
    char str[LWPRINTF_CFG_SUPPORT_LONG_LONG ? 22 : 11];

    /*
     * Check for corner cases
     *
     * - Print "nan" if number is not valid
     * - Print negative infinity if number is less than absolute minimum
     * - Print negative infinity if number is less than -FLOAT_MAX_B_ENG and engineering mode is disabled
     * - Print positive infinity if number is greater than absolute minimum
     * - Print positive infinity if number is greater than FLOAT_MAX_B_ENG and engineering mode is disabled
     * - Go to engineering mode if it is enabled and `in_num < -FLOAT_MAX_B_ENG` or `in_num > FLOAT_MAX_B_ENG`
     */
    if (in_num != in_num) {
        return prv_out_str(lwi, lwi->m.flags.uc ? "NAN" : "nan", 3);
    } else if (in_num < -DBL_MAX
#if !LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING
               || in_num < -FLOAT_MAX_B_ENG
#endif /* !LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING */
    ) {
        return prv_out_str(lwi, lwi->m.flags.uc ? "-INF" : "-inf", 4);
    } else if (in_num > DBL_MAX
#if !LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING
               || in_num > FLOAT_MAX_B_ENG
#endif /* !LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING */
    ) {
        char str[5], *s_ptr = str;
        if (lwi->m.flags.plus) {
            *s_ptr++ = '+';
        }
        strcpy(s_ptr, lwi->m.flags.uc ? "INF" : "inf");
        return prv_out_str(lwi, str, lwi->m.flags.plus ? 4 : 3);
#if LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING
    } else if ((in_num < -FLOAT_MAX_B_ENG || in_num > FLOAT_MAX_B_ENG) && def_type != 'g') {
        lwi->m.type = def_type = 'e'; /* Go to engineering mode */
#endif                                /* LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING */
    }

    /* Check sign of the number */
    SIGNED_CHECK_NEGATIVE(lwi, in_num);
    orig_num = in_num;

#if LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING
    /* Engineering mode check for number of exponents */
    if (def_type == 'e' || def_type == 'g'
        || in_num > (double)(powers_of_10[LWPRINTF_ARRAYSIZE(powers_of_10) - 1])) { /* More vs what float can hold */
        if (lwi->m.type != 'g') {
            lwi->m.type = 'e';
        }

        /* Normalize number to be between 0 and 1 and count decimals for exponent */
        if (in_num < 1) {
            for (exp_cnt = 0; in_num < 1 && in_num > 0; in_num *= 10, --exp_cnt) {}
        } else {
            for (exp_cnt = 0; in_num >= 10; in_num /= 10, ++exp_cnt) {}
        }
    }
#endif /* LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING */

    /* Check precision data */
    chosen_precision = lwi->m.precision; /* This is default value coming from app */
    if (lwi->m.precision >= (int)LWPRINTF_ARRAYSIZE(powers_of_10)) {
        lwi->m.precision = (int)LWPRINTF_ARRAYSIZE(powers_of_10) - 1; /* Limit to maximum precision */
        /*
         * Precision is lower than the one selected by app (or user).
         * It means that we have to append ending zeros for precision when printing data
         */
    } else if (!lwi->m.flags.precision) {
        lwi->m.precision = LWPRINTF_CFG_FLOAT_DEFAULT_PRECISION; /* Default precision when not used */
        chosen_precision = lwi->m.precision;                     /* There was no precision, update chosen precision */
    } else if (lwi->m.flags.precision && lwi->m.precision == 0) {
#if LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING
        /* Precision must be set to 1 if set to 0 by default */
        if (def_type == 'g') {
            lwi->m.precision = 1;
        }
#endif /* LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING */
    }

    /* Check if type is g and decide if final output should be 'f' or 'e' */
    /*
     * For 'g/G' specifier
     *
     * A double argument representing a floating-point number is converted
     * in style 'f' or 'e' (or in style 'F' or 'E' in the case of a 'G' conversion specifier),
     * depending on the value converted and the precision.
     * Let 'P' equal the precision if nonzero, '6' if the precision is omitted, or '1' if the precision is zero.
     * Then, if a conversion with style 'E' would have an exponent of 'X':
     *
     * if 'P > X >= -4', the conversion is with style 'f' (or 'F') and precision 'P - (X + 1)'.
     * otherwise, the conversion is with style 'e' (or 'E') and precision 'P - 1'.
     *
     * Finally, unless the '#' flag is used,
     * any trailing zeros are removed from the fractional portion of the result
     * and the decimal-point character is removed if there is no fractional portion remaining.
     *
     * A double argument representing an infinity or 'NaN' is converted in the style of an 'f' or 'F' conversion specifier.
     */

    /* Calculate data for number */
    prv_calculate_dbl_num_data(lwi, &dblnum, def_type == 'e' ? in_num : orig_num, def_type);

#if LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING
    /* Set type G */
    if (def_type == 'g') {
        /* As per standard to decide level of precision */
        if (exp_cnt >= -4 && exp_cnt < lwi->m.precision) {
            if (lwi->m.precision > exp_cnt) {
                lwi->m.precision -= exp_cnt + 1;
                chosen_precision -= exp_cnt + 1;
            } else {
                lwi->m.precision = 0;
                chosen_precision = 0;
            }
            lwi->m.type = 'f';
            in_num = orig_num;
        } else {
            lwi->m.type = 'e';
            if (lwi->m.precision > 0) {
                --lwi->m.precision;
                --chosen_precision;
            }
        }
        prv_calculate_dbl_num_data(lwi, &dblnum, in_num, def_type);
    }
#endif /* LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING */

    /* Set number of digits to display */
    digits_cnt = dblnum.digits_cnt_integer_part;
    if (0) {
#if LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING
    } else if (def_type == 'g' && lwi->m.precision > 0) {
        digits_cnt += dblnum.digits_cnt_decimal_part_useful;
        if (dblnum.digits_cnt_decimal_part_useful > 0) {
            ++digits_cnt;
        }
#endif /* LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING */
    } else {
        if (chosen_precision > 0 && lwi->m.flags.precision) {
            /* Add precision digits + dot separator */
            digits_cnt += chosen_precision + 1;
        }
    }

#if LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING
    /* Increase number of digits to display */
    if (lwi->m.type == 'e') {
        /* Format is +Exxx, so add 4 or 5 characters (max is 307, min is 00 for exponent) */
        digits_cnt += 4 + (exp_cnt >= 100 || exp_cnt <= -100);
    }
#endif /* LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING */

    /* Output strings */
    prv_out_str_before(lwi, digits_cnt);

    /* Output integer part of number */
    if (dblnum.integer_part == 0) {
        lwi->out_fn(lwi, '0');
    } else {
        for (i = 0; dblnum.integer_part > 0; dblnum.integer_part /= 10, ++i) {
            str[i] = (char)'0' + (char)(dblnum.integer_part % 10);
        }
        for (; i > 0; --i) {
            lwi->out_fn(lwi, str[i - 1]);
        }
    }

    /* Output decimal part */
    if (lwi->m.precision > 0) {
        int x;
        if (dblnum.digits_cnt_decimal_part_useful > 0) {
            lwi->out_fn(lwi, '.');
        }
        for (i = 0; dblnum.decimal_part > 0; dblnum.decimal_part /= 10, ++i) {
            str[i] = (char)'0' + (char)(dblnum.decimal_part % 10);
        }

        /* Output relevant zeros first, string to print is opposite way */
#if LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING
        if (def_type == 'g') {
            /* TODO: This is to be checked */
            for (x = 0; x < (lwi->m.precision - i) && dblnum.digits_cnt_decimal_part_useful > 0;
                 ++x, --dblnum.digits_cnt_decimal_part_useful) {
                lwi->out_fn(lwi, '0');
            }
        } else
#endif /* LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING */
        {
            for (x = i; x < lwi->m.precision; ++x) {
                lwi->out_fn(lwi, '0');
            }
        }

        /* Now print string itself */
        for (; i > 0; --i) {
            lwi->out_fn(lwi, str[i - 1]);
#if LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING
            if (def_type == 'g' && --dblnum.digits_cnt_decimal_part_useful == 0) {
                break;
            }
#endif /* LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING */
        }

        /* Print ending zeros if selected precision is bigger than maximum supported */
        if (def_type != 'g') {
            for (; x < chosen_precision; ++x) {
                lwi->out_fn(lwi, '0');
            }
        }
    }

#if LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING
    /* Engineering mode output, add exponent part */
    if (lwi->m.type == 'e') {
        lwi->out_fn(lwi, lwi->m.flags.uc ? 'E' : 'e');
        lwi->out_fn(lwi, exp_cnt >= 0 ? '+' : '-');
        if (exp_cnt < 0) {
            exp_cnt = -exp_cnt;
        }
        if (exp_cnt >= 100) {
            lwi->out_fn(lwi, (char)'0' + (char)(exp_cnt / 100));
            exp_cnt /= 100;
        }
        lwi->out_fn(lwi, (char)'0' + (char)(exp_cnt / 10));
        lwi->out_fn(lwi, (char)'0' + (char)(exp_cnt % 10));
    }
#endif /* LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING */
    prv_out_str_after(lwi, digits_cnt);

    return 1;
}

#endif /* LWPRINTF_CFG_SUPPORT_TYPE_FLOAT */

/**
 * \brief           Process format string and parse variable parameters
 * \param[in,out]   lwi: LwPRINTF internal instance
 * \param[in]       arg: Variable parameters list
 * \return          `1` on success, `0` otherwise
 */
static uint8_t
prv_format(lwprintf_int_t* lwi, va_list arg) {
    uint8_t detected = 0;
    const char* fmt = lwi->fmt;

#if LWPRINTF_CFG_OS && !LWPRINTF_CFG_OS_MANUAL_PROTECT
    if (IS_PRINT_MODE(lwi) &&                                /* OS protection only for print */
        (!lwprintf_sys_mutex_isvalid(&lwi->lwobj->mutex)     /* Invalid mutex handle */
         || !lwprintf_sys_mutex_wait(&lwi->lwobj->mutex))) { /* Cannot acquire mutex */
        return 0;
    }
#endif /* LWPRINTF_CFG_OS && !LWPRINTF_CFG_OS_MANUAL_PROTECT */

    while (fmt != NULL && *fmt != '\0') {
        /* Check if we should stop processing */
        if (lwi->is_print_cancelled) {
            break;
        }

        /* Detect beginning */
        if (*fmt != '%') {
            lwi->out_fn(lwi, *fmt); /* Output character */
            ++fmt;
            continue;
        }
        ++fmt;
        memset(&lwi->m, 0x00, sizeof(lwi->m)); /* Reset structure */

        /* Parse format */
        /* %[flags][width][.precision][length]type */
        /* Go to https://docs.majerle.eu for more info about supported features */

        /* Check [flags] */
        /* It can have multiple flags in any order */
        detected = 1;
        do {
            switch (*fmt) {
                case '-': lwi->m.flags.left_align = 1; break;
                case '+': lwi->m.flags.plus = 1; break;
                case ' ': lwi->m.flags.space = 1; break;
                case '0': lwi->m.flags.zero = 1; break;
                case '\'': lwi->m.flags.thousands = 1; break;
                case '#': lwi->m.flags.alt = 1; break;
                default: detected = 0; break;
            }
            if (detected) {
                ++fmt;
            }
        } while (detected);

        /* Check [width] */
        lwi->m.width = 0;
        if (CHARISNUM(*fmt)) { /* Fixed width check */
            /* If number is negative, it has been captured from previous step (left align) */
            lwi->m.width = prv_parse_num(&fmt); /* Number from string directly */
        } else if (*fmt == '*') {               /* Or variable check */
            const int w = (int)va_arg(arg, int);
            if (w < 0) {
                lwi->m.flags.left_align = 1; /* Negative width means left aligned */
                lwi->m.width = -w;
            } else {
                lwi->m.width = w;
            }
            ++fmt;
        }

        /* Check [.precision] */
        lwi->m.precision = 0;
        if (*fmt == '.') { /* Precision flag is detected */
            lwi->m.flags.precision = 1;
            if (*++fmt == '*') { /* Variable check */
                const int pr = (int)va_arg(arg, int);
                lwi->m.precision = pr > 0 ? pr : 0;
                ++fmt;
            } else if (CHARISNUM(*fmt)) { /* Directly in the string */
                lwi->m.precision = prv_parse_num(&fmt);
            }
        }

        /* Check [length] */
        detected = 1;
        switch (*fmt) {
            case 'h':
                lwi->m.flags.char_short = 1;     /* Single h detected */
                if (*++fmt == 'h') {             /* Does it follow by another h? */
                    lwi->m.flags.char_short = 2; /* Second h detected */
                    ++fmt;
                }
                break;
            case 'l':
                lwi->m.flags.longlong = 1;     /* Single l detected */
                if (*++fmt == 'l') {           /* Does it follow by another l? */
                    lwi->m.flags.longlong = 2; /* Second l detected */
                    ++fmt;
                }
                break;
            case 'L': break;
            case 'z':
                lwi->m.flags.sz_t = 1; /* Size T flag */
                ++fmt;
                break;
            case 'j':
                lwi->m.flags.umax_t = 1; /* uintmax_t flag */
                ++fmt;
                break;
            case 't': break;
            default: detected = 0;
        }

        /* Check type */
        lwi->m.type = *fmt + (char)((*fmt >= 'A' && *fmt <= 'Z') ? 0x20 : 0x00);
        if (*fmt >= 'A' && *fmt <= 'Z') {
            lwi->m.flags.uc = 1;
        }
        switch (*fmt) {
            case 'a':
            case 'A':
                /* Double in hexadecimal notation */
                (void)va_arg(arg, double);      /* Read argument to ignore it and move to next one */
                prv_out_str_raw(lwi, "NaN", 3); /* Print string */
                break;
            case 'c': lwi->out_fn(lwi, (char)va_arg(arg, int)); break;
#if LWPRINTF_CFG_SUPPORT_TYPE_INT
            case 'd':
            case 'i': {
                /* Check for different length parameters */
                lwi->m.base = 10;
                if (lwi->m.flags.longlong == 0) {
                    prv_signed_int_to_str(lwi, (signed int)va_arg(arg, signed int));
                } else if (lwi->m.flags.longlong == 1) {
                    prv_signed_long_int_to_str(lwi, (signed long int)va_arg(arg, signed long int));
#if LWPRINTF_CFG_SUPPORT_LONG_LONG
                } else if (lwi->m.flags.longlong == 2) {
                    prv_signed_longlong_int_to_str(lwi, (signed long long int)va_arg(arg, signed long long int));
#endif /* LWPRINTF_CFG_SUPPORT_LONG_LONG */
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
                    lwi->m.base = 2;
                } else if (*fmt == 'o') {
                    lwi->m.base = 8;
                } else if (*fmt == 'u') {
                    lwi->m.base = 10;
                } else if (*fmt == 'x' || *fmt == 'X') {
                    lwi->m.base = 16;
                }
                lwi->m.flags.space = 0; /* Space flag has no meaning here */

                /* Check for different length parameters */
                if (0) {

                } else if (lwi->m.flags.sz_t) {
                    prv_sizet_to_str(lwi, (size_t)va_arg(arg, size_t));
                } else if (lwi->m.flags.umax_t) {
                    prv_umaxt_to_str(lwi, (uintmax_t)va_arg(arg, uintmax_t));
                } else if (lwi->m.flags.longlong == 0 || lwi->m.base == 2) {
                    unsigned int v;
                    switch (lwi->m.flags.char_short) {
                        case 2: v = (unsigned int)((unsigned char)va_arg(arg, unsigned int)); break;
                        case 1: v = (unsigned int)((unsigned short int)va_arg(arg, unsigned int)); break;
                        default: v = (unsigned int)((unsigned int)va_arg(arg, unsigned int)); break;
                    }
                    prv_unsigned_int_to_str(lwi, v);
                } else if (lwi->m.flags.longlong == 1) {
                    prv_unsigned_long_int_to_str(lwi, (unsigned long int)va_arg(arg, unsigned long int));
#if LWPRINTF_CFG_SUPPORT_LONG_LONG
                } else if (lwi->m.flags.longlong == 2) {
                    prv_unsigned_longlong_int_to_str(lwi, (unsigned long long int)va_arg(arg, unsigned long long int));
#endif /* LWPRINTF_CFG_SUPPORT_LONG_LONG */
                }
                break;
#endif /* LWPRINTF_CFG_SUPPORT_TYPE_INT */
#if LWPRINTF_CFG_SUPPORT_TYPE_STRING
            case 's': {
                const char* b = va_arg(arg, const char*);
                /*
                 * Calculate length of the string:
                 *
                 * - If precision is given, max len is up to precision value
                 * - if user selects write to buffer, go up to buffer size (-1 actually, but handled by write function)
                 * - Otherwise use max available system length
                 */
                prv_out_str(lwi, b,
                            strnlen(b, lwi->m.flags.precision ? (size_t)lwi->m.precision
                                                              : (lwi->buff != NULL ? lwi->buff_size : SIZE_MAX)));
                break;
            }
#endif /* LWPRINTF_CFG_SUPPORT_TYPE_STRING */
#if LWPRINTF_CFG_SUPPORT_TYPE_POINTER
            case 'p': {
                lwi->m.base = 16;      /* Go to hex format */
                lwi->m.flags.uc = 0;   /* Uppercase characters */
                lwi->m.flags.zero = 1; /* Zero padding */
                lwi->m.width =
                    sizeof(uintptr_t) * 2; /* Number is in hex format and byte is represented with 2 letters */

                prv_uintptr_to_str(lwi, (uintptr_t)va_arg(arg, uintptr_t));
                break;
            }
#endif /* LWPRINTF_CFG_SUPPORT_TYPE_POINTER */
#if LWPRINTF_CFG_SUPPORT_TYPE_FLOAT
            case 'f':
            case 'F':
#if LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING
            case 'e':
            case 'E':
            case 'g':
            case 'G':
#endif /* LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING */
                /* Double number in different format. Final output depends on type of format */
                prv_double_to_str(lwi, (double)va_arg(arg, double));
                break;
#endif /* LWPRINTF_CFG_SUPPORT_TYPE_FLOAT */
            case 'n': {
                int* ptr = (void*)va_arg(arg, int*);
                *ptr = (int)lwi->n_len; /* Write current length */

                break;
            }
            case '%': lwi->out_fn(lwi, '%'); break;
#if LWPRINTF_CFG_SUPPORT_TYPE_BYTE_ARRAY
            /*
             * This is to print unsigned-char formatted pointer in hex string
             *
             * char arr[] = {0, 1, 2, 3, 255};
             * "%5K" would produce 00010203FF
             */
            case 'k':
            case 'K': {
                unsigned char* ptr =
                    (void*)va_arg(arg, unsigned char*); /* Get input parameter as unsigned char pointer */
                int len = lwi->m.width, full_width;
                uint8_t is_space = lwi->m.flags.space == 1;

                if (ptr == NULL || len == 0) {
                    break;
                }

                lwi->m.flags.zero = 1;  /* Prepend with zeros if necessary */
                lwi->m.width = 0;       /* No width parameter */
                lwi->m.base = 16;       /* Hex format */
                lwi->m.flags.space = 0; /* Delete any flag for space */

                /* Full width of digits to print */
                full_width = len * (2 + (int)is_space);
                if (is_space && full_width > 0) {
                    --full_width; /* Remove space after last number */
                }

                /* Output byte by byte w/o hex prefix */
                prv_out_str_before(lwi, full_width);
                for (int i = 0; i < len; ++i, ++ptr) {
                    uint8_t d;

                    d = (*ptr >> 0x04) & 0x0F; /* Print MSB */
                    lwi->out_fn(lwi, (char)(d) + (char)(d >= 10 ? ((lwi->m.flags.uc ? 'A' : 'a') - 10) : '0'));
                    d = *ptr & 0x0F; /* Print LSB */
                    lwi->out_fn(lwi, (char)(d) + (char)(d >= 10 ? ((lwi->m.flags.uc ? 'A' : 'a') - 10) : '0'));

                    if (is_space && i < (len - 1)) {
                        lwi->out_fn(lwi, ' '); /* Generate space between numbers */
                    }
                }
                prv_out_str_after(lwi, full_width);
                break;
            }
#endif /* LWPRINTF_CFG_SUPPORT_TYPE_BYTE_ARRAY */
            default: lwi->out_fn(lwi, *fmt);
        }
        ++fmt;
    }
    lwi->out_fn(lwi, '\0'); /* Output last zero number */
#if LWPRINTF_CFG_OS && !LWPRINTF_CFG_OS_MANUAL_PROTECT
    if (IS_PRINT_MODE(lwi)) { /* Mutex only for print operation */
        lwprintf_sys_mutex_release(&lwi->lwobj->mutex);
    }
#endif /* LWPRINTF_CFG_OS && !LWPRINTF_CFG_OS_MANUAL_PROTECT */
    return 1;
}

/**
 * \brief           Initialize LwPRINTF instance
 * \param[in,out]   lwobj: LwPRINTF working instance
 * \param[in]       out_fn: Output function used for print operation.
 *                      When set to `NULL`, direct print to stream functions won't work
 *                      and will return error if called by the application.
 *                      Also, system mutex for this specific instance won't be called
 *                      as system mutex isn't needed. All formatting functions (with print being an exception)
 *                      are thread safe. Library utilizes stack-based variables
 * \return          `1` on success, `0` otherwise
 */
uint8_t
lwprintf_init_ex(lwprintf_t* lwobj, lwprintf_output_fn out_fn) {
    LWPRINTF_GET_LWOBJ(lwobj)->out_fn = out_fn;

#if LWPRINTF_CFG_OS
    /* Create system mutex, but only if user selected to ever use print mode */
    if (out_fn != NULL
        && (lwprintf_sys_mutex_isvalid(&LWPRINTF_GET_LWOBJ(lwobj)->mutex)
            || !lwprintf_sys_mutex_create(&LWPRINTF_GET_LWOBJ(lwobj)->mutex))) {
        return 0;
    }
#endif /* LWPRINTF_CFG_OS */
    return 1;
}

/**
 * \brief           Print formatted data from variable argument list to the output
 * \param[in,out]   lwobj: LwPRINTF instance. Set to `NULL` to use default instance
 * \param[in]       format: C string that contains the text to be written to output
 * \param[in]       arg: A value identifying a variable arguments list initialized with `va_start`.
 *                      `va_list` is a special type defined in `<cstdarg>`.
 * \return          The number of characters that would have been written if `n` had been sufficiently large,
 *                      not counting the terminating null character.
 */
int
lwprintf_vprintf_ex(lwprintf_t* const lwobj, const char* format, va_list arg) {
    lwprintf_int_t fobj = {
        .lwobj = LWPRINTF_GET_LWOBJ(lwobj),
        .out_fn = prv_out_fn_print,
        .fmt = format,
        .buff = NULL,
        .buff_size = 0,
    };
    /* For direct print, output function must be set by user */
    if (fobj.lwobj->out_fn == NULL) {
        return 0;
    }
    if (prv_format(&fobj, arg)) {
        return (int)fobj.n_len;
    }
    return 0;
}

/**
 * \brief           Print formatted data to the output
 * \param[in,out]   lwobj: LwPRINTF instance. Set to `NULL` to use default instance
 * \param[in]       format: C string that contains the text to be written to output
 * \param[in]       ...: Optional arguments for format string
 * \return          The number of characters that would have been written if `n` had been sufficiently large,
 *                      not counting the terminating null character.
 */
int
lwprintf_printf_ex(lwprintf_t* const lwobj, const char* format, ...) {
    va_list valist;
    int n_len;

    va_start(valist, format);
    n_len = lwprintf_vprintf_ex(lwobj, format, valist);
    va_end(valist);

    return n_len;
}

/**
 * \brief           Write formatted data from variable argument list to sized buffer
 * \param[in,out]   lwobj: LwPRINTF instance. Set to `NULL` to use default instance
 * \param[in]       s_out: Pointer to a buffer where the resulting C-string is stored.
 *                      The buffer should have a size of at least `n` characters
 * \param[in]       n_maxlen: Maximum number of bytes to be used in the buffer.
 *                      The generated string has a length of at most `n - 1`,
 *                      leaving space for the additional terminating null character
 * \param[in]       format: C string that contains a format string that follows the same specifications as format in printf
 * \param[in]       arg: A value identifying a variable arguments list initialized with `va_start`.
 *                      `va_list` is a special type defined in `<cstdarg>`.
 * \return          The number of characters that would have been written if `n` had been sufficiently large,
 *                      not counting the terminating null character.
 */
int
lwprintf_vsnprintf_ex(lwprintf_t* const lwobj, char* s_out, size_t n_maxlen, const char* format, va_list arg) {
    lwprintf_int_t fobj = {
        .lwobj = LWPRINTF_GET_LWOBJ(lwobj),
        .out_fn = prv_out_fn_write_buff,
        .fmt = format,
        .buff = s_out,
        .buff_size = n_maxlen,
    };
    if (prv_format(&fobj, arg)) {
        return (int)fobj.n_len;
    }
    return 0;
}

/**
 * \brief           Write formatted data from variable argument list to sized buffer
 * \param[in,out]   lwobj: LwPRINTF instance. Set to `NULL` to use default instance
 * \param[in]       s_out: Pointer to a buffer where the resulting C-string is stored.
 *                      The buffer should have a size of at least `n` characters
 * \param[in]       n_maxlen: Maximum number of bytes to be used in the buffer.
 *                      The generated string has a length of at most `n - 1`,
 *                      leaving space for the additional terminating null character
 * \param[in]       format: C string that contains a format string that follows the same specifications as format in printf
 * \param[in]       ...: Optional arguments for format string
 * \return          The number of characters that would have been written if `n` had been sufficiently large,
 *                      not counting the terminating null character.
 */
int
lwprintf_snprintf_ex(lwprintf_t* const lwobj, char* s_out, size_t n_maxlen, const char* format, ...) {
    va_list valist;
    int len;

    va_start(valist, format);
    len = lwprintf_vsnprintf_ex(lwobj, s_out, n_maxlen, format, valist);
    va_end(valist);

    return len;
}

#if LWPRINTF_CFG_OS_MANUAL_PROTECT || __DOXYGEN__

/**
 * \brief           Manually enable mutual exclusion
 * \param[in,out]   lwobj: LwPRINTF instance. Set to `NULL` to use default instance
 * \return          `1` if protected, `0` otherwise
 */
uint8_t
lwprintf_protect_ex(lwprintf_t* const lwobj) {
    lwprintf_t* obj = LWPRINTF_GET_LWOBJ(lwobj);
    return obj->out_fn != NULL && lwprintf_sys_mutex_isvalid(&obj->mutex) && lwprintf_sys_mutex_wait(&obj->mutex);
}

/**
 * \brief           Manually disable mutual exclusion
 * \param[in,out]   lwobj: LwPRINTF instance. Set to `NULL` to use default instance
 * \return          `1` if protection disabled, `0` otherwise
 */
uint8_t
lwprintf_unprotect_ex(lwprintf_t* const lwobj) {
    lwprintf_t* obj = LWPRINTF_GET_LWOBJ(lwobj);
    return obj->out_fn != NULL && lwprintf_sys_mutex_release(&obj->mutex);
}

#endif /* LWPRINTF_CFG_OS_MANUAL_PROTECT || __DOXYGEN__ */
