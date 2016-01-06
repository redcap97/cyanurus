/*
Copyright 2014 Akira Midorikawa

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "uart.h"

#define UART0 ((volatile uint32_t*)0x10009000)

#define UARTFR      0x06
#define UARTFR_TXFF 0x20
#define UARTFR_RXFE 0x10

#define UARTIMSC        0x0e
#define UARTIMSC_RIMIM  (1 << 0)
#define UARTIMSC_CTSMIM (1 << 1)
#define UARTIMSC_DCDMIM (1 << 2)
#define UARTIMSC_DSRMIM (1 << 3)
#define UARTIMSC_RXIM   (1 << 4)
#define UARTIMSC_TXIM   (1 << 5)
#define UARTIMSC_RTIM   (1 << 6)
#define UARTIMSC_FEIM   (1 << 7)
#define UARTIMSC_PEIM   (1 << 8)
#define UARTIMSC_BEIM   (1 << 9)
#define UARTIMSC_OEIM   (1 << 10)

int uart_can_recv(void) {
  return !(*(UART0 + UARTFR) & UARTFR_RXFE);
}

int uart_can_send(void) {
  return !(*(UART0 + UARTFR) & UARTFR_TXFF);
}

void uart_set_read_interrupt(void) {
  *(UART0 + UARTIMSC) = UARTIMSC_RXIM | UARTIMSC_RTIM;
}

void uart_set_write_interrupt(void) {
  *(UART0 + UARTIMSC) = UARTIMSC_TXIM;
}

void uart_clear_interrupt(void) {
  *(UART0 + UARTIMSC) = 0;
}

int uart_recv(void) {
  while (!uart_can_recv());
  return (*UART0 & 0xff);
}

int uart_send(int c) {
  while (!uart_can_send());
  *UART0 = (c & 0xff);
  return 0;
}

int uart_getc(void) {
  int c = uart_recv();
  c = (c == '\r') ? '\n' : c;
  return c;
}

int uart_putc(int c) {
  if (c == '\n') {
    uart_send('\r');
  }
  uart_send(c);
  return 0;
}

ssize_t uart_read(void *data, size_t size) {
  size_t i;
  char ch;
  char *buf = data;

  for (i = 0; i < size; ++i) {
    ch = uart_getc();
    buf[i] = ch;

    if (ch == '\n') {
      return i + 1;
    }
  }

  return size;
}

ssize_t uart_write(const void *data, size_t size) {
  size_t i;
  const char *buf = data;

  for (i = 0; i < size; ++i) {
    uart_putc(buf[i]);
  }

  return size;
}
