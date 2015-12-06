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

#include <fs/block.c>

#include "test.h"
#include "fs/block.t"

static void setup(void) {
  page_init();
  fs_block_init();
}

static void assert_pattern(int pat, char *start, char *end) {
  char *current;

  TEST_ASSERT(start < end);

  for (current = start; current < end; ++current) {
    TEST_ASSERT(*current == pat);
  }
}

TEST(test_fs_block_read) {
  int i;
  char buf[BLOCK_SIZE];
  setup();

  TEST_ASSERT(list_length(&used_blocks) == 0);

  for (i = 1; i < 8; ++i) {
    fs_block_read(i, buf);
    TEST_ASSERT(list_length(&used_blocks) == i);

    fs_block_read(i, buf);
    TEST_ASSERT(list_length(&used_blocks) == i);
  }
}

TEST(test_fs_block_write) {
  int i;
  char buf[BLOCK_SIZE];
  setup();

  TEST_ASSERT(list_length(&used_blocks) == 0);

  for (i = 1; i < 8; ++i) {
    if (i & 0x1) {
      fs_block_read(i, buf);
      TEST_ASSERT(list_length(&used_blocks) == i);
    }

    memset(buf, 0xff, BLOCK_SIZE);
    fs_block_write(i, buf);

    TEST_ASSERT(list_length(&used_blocks) == i);

    memset(buf, 0, BLOCK_SIZE);
    fs_block_read(i, buf);

    TEST_ASSERT(list_length(&used_blocks) == i);
    assert_pattern(0xff, buf, buf + BLOCK_SIZE);
  }
}
