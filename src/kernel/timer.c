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

#include "timer.h"
#include "gic.h"
#include "lib/type.h"

#define TIMER0         ((volatile uint32_t*)0x10011000)
#define TIMER_VALUE    0x1
#define TIMER_CONTROL  0x2
#define TIMER_INTCLR   0x3
#define TIMER_MIS      0x5

#define TIMER_EN       0x80
#define TIMER_PERIODIC 0x40
#define TIMER_INTEN    0x20
#define TIMER_32BIT    0x02
#define TIMER_ONESHOT  0x01

/* 1MHz timer */
#define TIMER_FREQUENCY 1000000

void timer_enable(void) {
  gic_enable_irq(IRQ_TIMER01);

  /* interrupt every 10ms */
  *TIMER0 = (uint32_t)(TIMER_FREQUENCY * 0.01);

  *(TIMER0 + TIMER_CONTROL) =
    TIMER_EN | TIMER_PERIODIC | TIMER_32BIT | TIMER_INTEN;
}

int timer_is_masked(void) {
  return *(TIMER0 + TIMER_MIS);
}

void timer_clear_interrupt(void) {
  *(TIMER0 + TIMER_INTCLR) = 1;
}
