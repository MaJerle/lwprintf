/**
 * \file            lwprintf_opts.h
 * \brief           LwPRINTF application options
 */

/*
 * Copyright (c) 2024 Tilen MAJERLE
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
 * This file is part of Lightweight stdio manager library.
 *
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 * Version:         v1.0.6
 */
#ifndef LWPRINTF_HDR_OPTS_H
#define LWPRINTF_HDR_OPTS_H

/* Rename this file to "lwprintf_opts.h" for your application */

#include "windows.h"

/*
 * Open "include/lwprintf/lwprintf_opt.h" and
 * copy & replace here settings you want to change values
 */
#define LWPRINTF_CFG_OS                1
#define LWPRINTF_CFG_OS_MUTEX_HANDLE   HANDLE

#define LWPRINTF_CFG_SUPPORT_LONG_LONG 1
#define LWPRINTF_CFG_OS_MANUAL_PROTECT 1

#endif /* LWPRINTF_HDR_OPTS_H */
