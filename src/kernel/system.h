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

#ifndef _CYANURUS_SYSTEM_H_
#define _CYANURUS_SYSTEM_H_

#include "lib/type.h"
#include "lib/unix.h"
#include "lib/extension.h"

#define SYSTEM_BUG_ON(expr)                      \
  if ((expr)) {                                  \
    system_bug_on(__FILE__, __LINE__, __func__); \
  }

void system_init(void);
void system_irq_handler(void);
void system_svc_handler(void);
void system_data_abort_handler(void);
noreturn void system_halt(void);
noreturn void system_bug_on(const char *file, unsigned int line, const char *func);
void system_sleep(void);
void system_shutdown(void);
int system_uname(struct utsname *uts);

// implemented in asm/lib.S
void system_dispatch(uint32_t context);
void system_suspend(uint32_t context);
void system_resume(uint32_t context);
void system_idle(void);

#endif
