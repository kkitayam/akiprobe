/* SPDX-License-Identifier: MIT
 *
 * Copyright (c) 2022 Koji KITAYAMA
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE. */

#ifndef _BSP_BOARD_H_
#define _BSP_BOARD_H_

#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "tusb.h"

void board_init(void);
uint32_t board_uart_set_baudrate(unsigned bit_rate);
int board_uart_read(uint8_t* buf, int len);
int board_uart_write(void const * buf, int len);
int board_swo_set_enabled(int enabled);
uint32_t board_swo_set_baudrate(unsigned bit_rate);
int board_swo_read(uint8_t* buf, int len);

#ifdef __cplusplus
}
#endif

#endif /* _BSP_BOARD_H_ */
