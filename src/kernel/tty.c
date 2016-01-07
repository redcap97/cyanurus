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

static struct process_waitq tty_read_waitq, tty_write_waitq;

static int tty_getc(void) {
  while (1) {
    if (!uart_can_recv()) {
      uart_set_interrupt(O_RDONLY);
      process_sleep(&tty_read_waitq);
      uart_clear_interrupt(O_RDONLY);
      continue;
    }
    return uart_getc();
  }
}

static int tty_putc(int c) {
  while (1) {
    if (!uart_can_send()) {
      uart_set_interrupt(O_WRONLY);
      process_sleep(&tty_write_waitq);
      uart_clear_interrupt(O_WRONLY);
      continue;
    }
    return uart_putc(c);
  }
}

static void wake_next(void) {
  process_wake(&tty_read_waitq);
  process_wake(&tty_write_waitq);
}

void tty_init(void) {
  gic_enable_irq(IRQ_UART0);

  process_waitq_init(&tty_read_waitq);
  process_waitq_init(&tty_write_waitq);
}

void tty_resume(void) {
  if (uart_can_recv()) {
    process_wake(&tty_read_waitq);
  }

  if (uart_can_send()) {
    process_wake(&tty_write_waitq);
  }
}

ssize_t tty_read(void *data, size_t size) {
  size_t i;
  char ch;
  char *buf = data;

  for (i = 0; i < size; ++i) {
    ch = tty_getc();

    tty_putc(ch);
    buf[i] = ch;

    if (ch == '\n') {
      size = i + 1;
      goto done;
    }
  }

done:
  wake_next();
  return size;
}

ssize_t tty_write(const void *data, size_t size) {
  size_t i;
  const char *buf = data;

  for (i = 0; i < size; ++i) {
    tty_putc(buf[i]);
  }

  wake_next();
  return size;
}
