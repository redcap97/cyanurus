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

#include "superblock.h"
#include "lib/string.h"
#include "block.h"
#include "logger.h"
#include "system.h"
#include "buddy.h"

#define SUPERBLOCK_ADDRESS 0x400

struct minix3_superblock superblock;

void superblock_init(void) {
  _page_cleanup_ struct page *page = buddy_alloc(BLOCK_SIZE);
  char *buf = page_address(page);

  SYSTEM_BUG_ON((SUPERBLOCK_ADDRESS + sizeof(struct minix3_superblock)) > BLOCK_SIZE);

  block_read(0, buf);
  memcpy(&superblock, buf + SUPERBLOCK_ADDRESS, sizeof(struct minix3_superblock));

  if (superblock.s_magic != SUPER_MAGIC_V3) {
    logger_fatal("invalid magic bytes: 0x%x", (uint32_t)superblock.s_magic);
    system_halt();
  }

  if (superblock.s_log_zone_size) {
    logger_fatal("blocks per zone must be zero: 0x%x", (uint32_t)superblock.s_log_zone_size);
    system_halt();
  }

  if (superblock.s_blocksize != BLOCK_SIZE) {
    logger_fatal("block size must be 4096: 0x%x", (uint32_t)superblock.s_blocksize);
    system_halt();
  }
}
