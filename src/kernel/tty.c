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

#include "tty.h"
#include "uart.h"
#include "gic.h"
#include "process.h"

static struct process_waitq tty_waitq;

void tty_init(void) {
  process_waitq_init(&tty_waitq);
  gic_enable_irq(IRQ_UART0);
}

ssize_t tty_read(void *data, size_t size) {
  size_t i;
  char ch;
  char *buf = data;

  for (i = 0; i < size; ++i) {
    ch = uart_getc();

    uart_putc(ch);
    buf[i] = ch;

    if (ch == '\n') {
      return i + 1;
    }
  }

  return size;
}

ssize_t tty_write(const void *data, size_t size) {
  return uart_write(data, size);
}
