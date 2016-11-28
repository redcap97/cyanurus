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
#include "lib/string.h"

void __aeabi_memset(void *buf, size_t size, int c) {
  memset(buf, c, size);
}

void __aeabi_memcpy(void *dest, const void *src, size_t size) {
  memcpy(dest, src, size);
}

void __aeabi_memclr(void *dest, size_t n) {
  memset(dest, 0, n);
}

void __aeabi_memclr8(void *dest, size_t n) {
  memset(dest, 0, n);
}

void __aeabi_ldiv0(void) {
  system_halt();
}

void __aeabi_idiv0(void) {
  system_halt();
}
