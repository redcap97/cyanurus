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

#include "block.h"
#include "lib/string.h"
#include "lib/list.h"
#include "buddy.h"
#include "page.h"
#include "slab.h"
#include "system.h"
#include "logger.h"
#include "mmc.h"

#define BLOCKS_PER_PAGE (PAGE_SIZE/BLOCK_SIZE)

struct block {
  block_index index;
  struct list next;
  struct page *page;
  void *data;
};

static struct slab_cache* block_cache;

static struct list used_blocks;
static struct list free_blocks;

static void prepare_blocks(void) {
  int i;
  char *address;
  struct page *page;
  struct block *block;

  page = buddy_alloc(PAGE_SIZE);
  address = page_address(page);

  for (i = 0; i < BLOCKS_PER_PAGE; ++i) {
    block = slab_cache_alloc(block_cache);
    memset(block, 0, sizeof(struct block));

    block->page = page;
    block->data = address + (i * BLOCK_SIZE);

    list_add(&free_blocks, &block->next);
  }
}

static struct block *get_block(block_index index) {
  struct block *block;

  list_foreach(block, &used_blocks, next) {
    if (block->index == index) {
      return block;
    }
  }

  if (list_empty(&free_blocks)) {
    prepare_blocks();
  }

  block = NULL;
  list_foreach(block, &free_blocks, next) {
    list_remove(&block->next);
    break;
  }

  if (!block) {
    logger_fatal("broken block cache");
    system_halt();
  }

  block->index = index;
  list_add(&used_blocks, &block->next);

  mmc_read(index * BLOCK_SIZE, BLOCK_SIZE, block->data);
  return block;
}

void block_init(void) {
  mmc_init();

  block_cache = slab_cache_create("block", sizeof(struct block));

  list_init(&used_blocks);
  list_init(&free_blocks);
}

void block_read(block_index index, void *data) {
  struct block *block = get_block(index);
  memcpy(data, block->data, BLOCK_SIZE);
}

void block_write(block_index index, const void *data) {
  struct block *block = get_block(index);
  memcpy(block->data, data, BLOCK_SIZE);
  mmc_write(index * BLOCK_SIZE, BLOCK_SIZE, block->data);
}
