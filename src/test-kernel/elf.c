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

#include <elf.c>

#include "test.h"
#include "elf.t"

#include "mmu.h"

#define INIT_PID  1
#define INIT_PATH "/sbin/init"

static void setup(void) {
  page_init();
  fs_init();

  mmu_init();
  mmu_enable();
}

static void assert_pattern(int pat, uint8_t *start, uint8_t *end) {
  uint8_t *current;

  TEST_ASSERT(start < end);

  for (current = start; current < end; ++current) {
    TEST_ASSERT(*current == pat);
  }
}

TEST(test_elf) {
  struct elf_executable executable;

  uint8_t *text, *data;
  uint32_t text_start, text_end;
  uint32_t data_start, data_end;
  uint32_t fill_start, fill_end;

  setup();
  elf_load(INIT_PATH, &executable);

  text = page_address(executable.text.page);
  data = page_address(executable.data.page);

  text_start = executable.text.addr;
  text_end   = executable.text.addr + executable.text.memory_size;
  data_start = executable.data.addr;
  data_end   = executable.data.addr + executable.data.memory_size;
  TEST_ASSERT(text_start <= text_end);
  TEST_ASSERT(text_end <= data_start);
  TEST_ASSERT(data_start <= data_end);

  fill_start = text_start - 0x1000;
  fill_end   = data_end   + 0x1000;
  TEST_ASSERT(fill_start < fill_end);

  mmu_alloc(INIT_PID, fill_start, fill_end - fill_start);
  mmu_set_ttb(INIT_PID);

  memset((char*)fill_start, 0xff, fill_end - fill_start);
  elf_copy(&executable);

  TEST_ASSERT(!memcmp(text, (void*)executable.text.addr, executable.text.memory_size));
  TEST_ASSERT(!memcmp(data, (void*)executable.data.addr, executable.data.file_size));

  assert_pattern(
    0x00,
    (uint8_t*)executable.data.addr + executable.data.file_size,
    (uint8_t*)executable.data.addr + executable.data.memory_size
  );

  assert_pattern(0xff, (uint8_t*)fill_start, (uint8_t*)text_start);
  assert_pattern(0xff, (uint8_t*)text_end,   (uint8_t*)data_start);
  assert_pattern(0xff, (uint8_t*)data_end,   (uint8_t*)fill_end);

  elf_release(&executable);
  TEST_ASSERT(!executable.text.page);
  TEST_ASSERT(!executable.data.page);
}
