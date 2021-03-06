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

#ifndef _CYANURUS_UART_H_
#define _CYANURUS_UART_H_

#include "lib/type.h"

int uart_can_recv(void);
int uart_can_send(void);
int uart_recv(void);
int uart_send(int c);
int uart_getc(void);
int uart_putc(int c);
ssize_t uart_read(void *data, size_t size);
ssize_t uart_write(const void *data, size_t size);
void uart_set_interrupt(int flags);
void uart_clear_interrupt(int flags);

#endif
