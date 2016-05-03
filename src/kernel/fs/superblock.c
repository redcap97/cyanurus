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

/* FIXME: error handling is broken */

#include "fs/superblock.h"
#include "lib/string.h"
#include "fs/block.h"

#define SUPERBLOCK_ADDRESS 0x400

struct minix3_superblock superblock;

void fs_superblock_init(void) {
  char buf[BLOCK_SIZE];

  fs_block_read(0, buf);
  memcpy(&superblock, buf + SUPERBLOCK_ADDRESS, sizeof(struct minix3_superblock));
}
