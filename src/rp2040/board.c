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

#include "RP2040.h"
#include "hardware/gpio.h"
#include "hardware/uart.h"

#include "board.h"

#define UART          uart1
#define UART_TX_PIN   4
#define UART_RX_PIN   5
#define SWO           uart0
#define SWO_RX_PIN    1

static struct {
  uint32_t baudrate;
} g_swo = {
  .baudrate = 115200,
};

void board_init(void)
{
  // UART
  gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
  gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
  uart_init(UART, 115200);
  gpio_set_pulls(UART_TX_PIN, false, false);
  gpio_pull_up(UART_RX_PIN);

  SystemCoreClockUpdate();
}

//--------------------------------------------------------------------+
// Board porting API
//--------------------------------------------------------------------+
static int _board_uart_read(uint8_t* buf, int len, uart_inst_t* uart)
{
  int i;
  for (i = 0; i < len && uart_is_readable(uart); ++i) {
    buf[i] = uart_getc(uart);
  }
  return i;
}

static int _board_uart_write(void const * buf, int len, uart_inst_t* uart)
{
  char const* p = (char const*)buf;
  int i;
  for (i = 0; i < len; ++i) {
    uart_putc(uart, *p++);
  }
  return i;
}

uint32_t board_uart_set_baudrate(unsigned bit_rate)
{
  return uart_set_baudrate(UART, bit_rate);
}

int board_uart_read(uint8_t* buf, int len)
{
  return _board_uart_read(buf, len, UART);
}

int board_uart_write(void const * buf, int len)
{
  return _board_uart_write(buf, len, UART);
}

int board_swo_set_enabled(int enabled)
{
  if (enabled) {
    gpio_pull_up(SWO_RX_PIN);
    gpio_set_function(SWO_RX_PIN, GPIO_FUNC_UART);
    uint32_t baudrate = uart_init(SWO, g_swo.baudrate);
    return (0 != baudrate) ? 1 : 0;
  } else {
    uart_deinit(SWO);
    gpio_set_function(SWO_RX_PIN, GPIO_FUNC_NULL);
    gpio_pull_down(SWO_RX_PIN);
    return 1;
  }
}

uint32_t board_swo_set_baudrate(unsigned bit_rate)
{
  g_swo.baudrate = bit_rate;
  return uart_set_baudrate(SWO, bit_rate);
}

int board_swo_read(uint8_t* buf, int len)
{
  return _board_uart_read(buf, len, SWO);
}
