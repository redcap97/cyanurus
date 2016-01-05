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

#include "system.h"
#include "page.h"
#include "fs.h"
#include "uart.h"
#include "mmu.h"
#include "syscall.h"
#include "process.h"
#include "timer.h"
#include "logger.h"
#include "gic.h"
#include "pipe.h"
#include "config.h"
#include "tty.h"
#include "lib/string.h"

#define SCTLR_V (1 << 13)

#define SYS_CFGCTRL      ((volatile uint32_t*)0x100000a4)
#define SYS_CFG_START    (1 << 31)
#define SYS_CFG_WRITE    (1 << 30)
#define SYS_CFG_SHUTDOWN (8 << 20)

static void enable_hight_vectors(void) {
  int sctlr;

  /* SCTLR */
  __asm__(
    "MRC p15, 0, %[sctlr], c1, c0, 0 \n\t"
    : [sctlr] "=r"(sctlr)
  );

  __asm__(
    "MCR p15, 0, %[sctlr], c1, c0, 0 \n\t"
    :
    : [sctlr] "r"(sctlr | SCTLR_V)
  );
}

void system_init(void) {
  enable_hight_vectors();
  gic_init();
  page_init();
  fs_init();
  tty_init();
  mmu_init();
  mmu_enable();
  timer_enable();
  pipe_init();
  process_init();
}

void system_irq_handler(void) {
  uint32_t irq = gic_interrupt_acknowledge();

  switch(irq) {
    case IRQ_TIMER01:
      if (timer_is_masked()) {
        timer_clear_interrupt();
      }
      break;

    default:
      logger_warn("unknown irq: %d", irq);
      break;
  }

  gic_end_of_interrupt(irq);
  process_switch();
}

void system_svc_handler(void) {
  syscall_handler();
}

noreturn void system_halt(void) {
  logger_fatal("system abort.");

  while (1) {
    system_sleep();
  }
}

noreturn void system_bug_on(const char *file, unsigned int line, const char *func) {
  logger_fatal("bug on @%s() (%s:%u)", func, file, line);
  system_halt();
}

void system_sleep(void) {
  __asm__ ("WFI");
}

void system_shutdown(void) {
  *SYS_CFGCTRL = SYS_CFG_START | SYS_CFG_WRITE | SYS_CFG_SHUTDOWN;
}

int system_uname(struct utsname *uts) {
  strcpy(uts->sysname, CYANURUS_SYSNAME);
  strcpy(uts->nodename, "(none)");
  strcpy(uts->release, CYANURUS_RELEASE);
  strcpy(uts->version, CYANURUS_VERSION);
  strcpy(uts->machine, CYANURUS_MACHINE);
  strcpy(uts->domainname, "(none)");

  return 0;
}
