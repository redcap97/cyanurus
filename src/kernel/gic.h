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

#ifndef _CYANURUS_GIC_H_
#define _CYANURUS_GIC_H_

#include "lib/type.h"

#define IRQ_TIMER01 34
#define IRQ_TIMER23 35

#define IRQ_UART0 37
#define IRQ_UART1 38
#define IRQ_UART2 39
#define IRQ_UART3 40

void gic_init(void);
void gic_enable_irq(int id);
uint32_t gic_interrupt_acknowledge(void);
void gic_end_of_interrupt(uint32_t irq);

#endif
