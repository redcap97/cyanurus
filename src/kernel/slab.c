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

#include "slab.h"
#include "buddy.h"
#include "logger.h"
#include "lib/string.h"
#include "system.h"

#define MAX_SLAB_NAME 32
#define SLAB_FREE_END 0xffffffff

#define SLAB_HEADER_SIZE (sizeof(struct slab_header) + sizeof(uint32_t))

struct slab_header {
  struct slab_header *next;
  uint32_t *free;
  uint8_t *object;
};

struct slab_cache {
  char name[MAX_SLAB_NAME];

  struct slab_cache *next;

  size_t slab_size;
  size_t object_size;

  struct slab_header *slabs_full;
  struct slab_header *slabs_partial;
};

static uint32_t slab_size_limits[PAGE_MAX_DEPTH] = {0};
struct slab_cache *free_cache_head = NULL;

static size_t slab_max_object_num(const struct slab_cache *cache) {
  return (cache->slab_size - SLAB_HEADER_SIZE) / (cache->object_size + sizeof(uint32_t));
}

static struct slab_header *slab_new(const struct slab_cache *cache) {
  struct slab_header *header;
  size_t i, max_object_num = slab_max_object_num(cache);

  header = (struct slab_header*)page_address(buddy_alloc(cache->slab_size));
  memset(header, 0, sizeof(struct slab_header));

  header->free = (void*)(header + 1);
  header->object = (void*)(header->free + max_object_num + 1);

  for (i = 0; i < max_object_num; ++i) {
    header->free[i] = i + 1;
  }
  header->free[max_object_num] = SLAB_FREE_END;

  return header;
}

static struct slab_cache *slab_cache_new(void) {
  size_t i;
  struct slab_cache *cache;

  if (!free_cache_head) {
    cache = memset(page_address(buddy_alloc(PAGE_SIZE)), 0, PAGE_SIZE);

    for (i = 0; i < (PAGE_SIZE / sizeof(struct slab_cache)) - 1; ++i) {
      cache[i].next = &cache[i+1];
    }

    free_cache_head = cache;
  }

  cache = free_cache_head;
  free_cache_head = cache->next;
  cache->next = NULL;

  return cache;
}

static void slab_cache_delete(struct slab_cache *cache) {
  memset(cache, 0, sizeof(struct slab_cache));

  cache->next = free_cache_head;
  free_cache_head = cache;
}

void slab_cache_init(void) {
  size_t i, page_size;

  for (i = 0; i < PAGE_MAX_DEPTH; ++i) {
    page_size = PAGE_SIZE * (1 << i);
    slab_size_limits[i] = (page_size / 8) - ((SLAB_HEADER_SIZE / 8) + 1) - sizeof(uint32_t);
  }

  free_cache_head = NULL;
}

struct slab_cache *slab_cache_create(const char *name, size_t size) {
  int i;
  struct slab_cache *cache;

  if (!size) {
    return NULL;
  }

  if (strlen(name) >= MAX_SLAB_NAME) {
    return NULL;
  }

  cache = slab_cache_new();

  strcpy(cache->name, name);
  cache->object_size = (size + 0x3) & ~0x3;

  for (i = 0; i < PAGE_MAX_DEPTH; ++i) {
    if (cache->object_size <= slab_size_limits[i]) {
      cache->slab_size = PAGE_SIZE * (1 << i);
      break;
    }
  }

  if (!cache->slab_size) {
    return NULL;
  }

  return cache;
}

void slab_cache_destroy(struct slab_cache *cache) {
  struct slab_header *header, *next_header;

  header = cache->slabs_full;
  while (header) {
    next_header = header->next;
    buddy_free(page_find_by_address(header));
    header = next_header;
  }

  header = cache->slabs_partial;
  while (header) {
    next_header = header->next;
    buddy_free(page_find_by_address(header));
    header = next_header;
  }

  slab_cache_delete(cache);
}

void *slab_cache_alloc(struct slab_cache *cache) {
  struct slab_header *header;
  uint32_t index, next_index, *free_list;

  header = cache->slabs_partial;

  if (!header) {
    header = slab_new(cache);
    cache->slabs_partial = header;
  }

  free_list = (void*)(header + 1);

  index = (size_t)(header->free - free_list);
  next_index = *header->free;

  free_list[index] = SLAB_FREE_END;
  header->free = &free_list[next_index];

  if (*header->free == SLAB_FREE_END) {
    cache->slabs_partial = header->next;

    if (cache->slabs_full) {
      header->next = cache->slabs_full;
    } else {
      header->next = NULL;
    }
    cache->slabs_full = header;
  }

  return (header->object + (cache->object_size * index));
}

void slab_cache_free(struct slab_cache *cache, void *obj) {
  uint32_t index, next_index, *free_list;

  struct page *page = page_find_head(page_find_by_address(obj));
  struct slab_header *header = (void*)page_address(page), *h, **hp;

  free_list = (void*)(header + 1);

  index = ((uint8_t*)obj - header->object) / cache->object_size;
  next_index = (size_t)(header->free - free_list);

  free_list[index] = next_index;
  header->free = &free_list[index];

  if (free_list[next_index] == SLAB_FREE_END) {
    hp = &cache->slabs_full;
    h = cache->slabs_full;

    while (h) {
      if (h == header) {
        *hp = h->next;

        h->next = cache->slabs_partial;
        cache->slabs_partial = h;

        return;
      }

      hp = &h->next;
      h = h->next;
    }

    logger_fatal("broken slab");
    system_halt();
  }
}
