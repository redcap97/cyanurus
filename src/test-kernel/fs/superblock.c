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

#include <fs/superblock.c>

#include "test.h"
#include "fs/superblock.t"

#include "page.h"
#include "logger.h"

static void setup(void) {
  page_init();
  block_init();
  fs_superblock_init();
}

TEST(test_fs_superblock_init) {
  setup();

  TEST_ASSERT(superblock.s_ninodes       > 0);
  TEST_ASSERT(superblock.s_imap_blocks   > 0);
  TEST_ASSERT(superblock.s_zmap_blocks   > 0);
  TEST_ASSERT(superblock.s_firstdatazone > 0);
  TEST_ASSERT(superblock.s_log_zone_size == 0);
  TEST_ASSERT(superblock.s_max_size      > 0);
  TEST_ASSERT(superblock.s_zones         > 0);
  TEST_ASSERT(superblock.s_magic         == SUPER_MAGIC_V3);
  TEST_ASSERT(superblock.s_blocksize     == 4096);
  TEST_ASSERT(superblock.s_disk_version  == 0);
}
