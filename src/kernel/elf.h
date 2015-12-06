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

#ifndef _CYANURUS_ELF_H_
#define _CYANURUS_ELF_H_

#include "lib/type.h"
#include "page.h"

struct elf_segment {
  struct page *page;
  uint32_t addr;
  uint32_t file_size;
  uint32_t memory_size;
};

struct elf_executable {
  uint32_t entry_point;

  struct elf_segment text;
  struct elf_segment data;
};

int elf_load(const char *path, struct elf_executable *executable);
void elf_copy(struct elf_executable *executable);
void elf_release(struct elf_executable *executable);

#endif
