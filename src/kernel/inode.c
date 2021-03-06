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

#include "inode.h"
#include "superblock.h"
#include "lib/string.h"
#include "lib/errno.h"
#include "slab.h"
#include "logger.h"
#include "system.h"
#include "buddy.h"

#define IMAP_BLOCKS     (superblock.s_imap_blocks)
#define IMAP_LIMITS     (superblock.s_ninodes)
#define ZMAP_BLOCKS     (superblock.s_zmap_blocks)
#define ZMAP_LIMITS     (superblock.s_zones - superblock.s_firstdatazone)
#define FIRST_DATA_ZONE (superblock.s_firstdatazone)

#define IMAP_ZONE_INDEX  2
#define INODE_ZONE_INDEX (IMAP_ZONE_INDEX + IMAP_BLOCKS + ZMAP_BLOCKS)

#define INODES_PER_BLOCK (BLOCK_SIZE / sizeof(struct minix2_inode))
#define ZONES_PER_BLOCK  (BLOCK_SIZE / sizeof(uint32_t))

static struct slab_cache *inode_cache;
static struct list inodes;

static struct inode *get_inode(inode_index index) {
  struct inode *inode;

  list_foreach(inode, &inodes, next) {
    if (inode->index == index) {
      return inode;
    }
  }

  inode = slab_cache_alloc(inode_cache);
  memset(inode, 0, sizeof(struct inode));

  inode->index = index;
  list_add(&inodes, &inode->next);

  return inode;
}

static void release_inode(struct inode *inode) {
  list_remove(&inode->next);
  slab_cache_free(inode_cache, inode);
}

static uint32_t mark_map(block_index start, block_index end, uint32_t limit) {
  int i, b;
  uint32_t index;
  block_index block;

  _page_cleanup_ struct page *page = buddy_alloc(BLOCK_SIZE);
  uint8_t *buf = page_address(page);

  for (block = start, index = 0; block < end; ++block) {
    block_read(block, buf);

    for(i = 0; i < BLOCK_SIZE; ++i) {
      if (buf[i] == 0xffL) {
        index += 8;
        continue;
      }

      for (b = 0; b < 8; ++b) {
        if (index + b >= limit) {
          return 0;
        }

        if (~buf[i] & (1 << b)) {
          buf[i] |= (1 << b);
          block_write(block, buf);
          return index + b;
        }
      }
    }
  }

  return 0;
}

static void unmark_map(uint32_t index, block_index start) {
  block_index block = start + ((index / 8) / BLOCK_SIZE);

  _page_cleanup_ struct page *page = buddy_alloc(BLOCK_SIZE);
  uint8_t *buf = page_address(page);

  block_read(block, buf);

  buf[(index / 8) % BLOCK_SIZE] &= ~(1 << (index % 8));
  block_write(block, buf);
}

static inode_index mark_imap(void) {
  block_index start = IMAP_ZONE_INDEX;
  block_index end   = start + IMAP_BLOCKS;

  return mark_map(start, end, IMAP_LIMITS);
}

static void unmark_imap(inode_index index) {
  block_index start = IMAP_ZONE_INDEX;
  return unmark_map(index, start);
}

static block_index mark_zmap(void) {
  block_index start = IMAP_ZONE_INDEX + IMAP_BLOCKS;
  block_index end   = start + ZMAP_BLOCKS;
  block_index index = mark_map(start, end, ZMAP_LIMITS);

  if (!index) {
    return 0;
  }

  return index + FIRST_DATA_ZONE - 1;
}

static void unmark_zmap(block_index index) {
  block_index start = IMAP_ZONE_INDEX + IMAP_BLOCKS;

  if (index < FIRST_DATA_ZONE) {
    return;
  }

  unmark_map(index - FIRST_DATA_ZONE + 1, start);
}

static void read_inode(inode_index index, struct minix2_inode *inode) {
  block_index inode_block;

  _page_cleanup_ struct page *page = buddy_alloc(BLOCK_SIZE);
  struct minix2_inode *inodes = page_address(page);

  index--;
  inode_block = index / INODES_PER_BLOCK;

  block_read(INODE_ZONE_INDEX + inode_block, inodes);
  memcpy(inode, inodes + (index % INODES_PER_BLOCK), sizeof(struct minix2_inode));
}

static void write_inode(inode_index index, const struct minix2_inode *inode) {
  block_index inode_block;

  _page_cleanup_ struct page *page = buddy_alloc(BLOCK_SIZE);
  struct minix2_inode *inodes = page_address(page);

  index--;
  inode_block = index / INODES_PER_BLOCK;

  block_read(INODE_ZONE_INDEX + inode_block, inodes);
  memcpy(inodes + (index % INODES_PER_BLOCK), inode, sizeof(struct minix2_inode));

  block_write(INODE_ZONE_INDEX + inode_block, inodes);
}

static int extend_zone(struct minix2_inode *inode, size_t size) {
  block_index block, start, end;
  block_index z0, z1, z2;

  _page_cleanup_ struct page *page = buddy_alloc(BLOCK_SIZE);
  uint32_t *zones = page_address(page);

  start = (inode->i_size / BLOCK_SIZE) + 1;
  end = (size / BLOCK_SIZE) + 1;

  if (end < start) {
    return 0;
  }

  for (block = start; block < end; ++block) {
    if (block <= 6) {
      z0 = block;
    } else if (block > 6 && block < (7 + ZONES_PER_BLOCK)) {
      z0 = 7;
    } else {
      z0 = 8;
    }

    if (!inode->i_zone[z0]) {
      inode->i_zone[z0] = mark_zmap();
      if (!inode->i_zone[z0]) {
        goto fail;
      }
    }

    switch (z0) {
      case 7:
        z1 = inode->i_zone[z0];
        z2 = block - 7;
        break;

      case 8:
        z1 = inode->i_zone[z0];
        z2 = (block - (7 + ZONES_PER_BLOCK)) / ZONES_PER_BLOCK;
        break;

      default:
        continue;
    }

    block_read(z1, zones);
    if (!zones[z2]) {
      zones[z2] = mark_zmap();
      if (!zones[z2]) {
        goto fail;
      }
      block_write(z1, zones);
    }

    switch(z0) {
      case 8:
        z1 = zones[z2];
        z2 = (block - (7 + ZONES_PER_BLOCK)) % ZONES_PER_BLOCK;
        break;

      default:
        continue;
    }

    block_read(z1, zones);
    if (!zones[z2]) {
      zones[z2] = mark_zmap();
      if (!zones[z2]) {
        goto fail;
      }
      block_write(z1, zones);
    }
  }

  inode->i_size = size;
  return 0;
fail:
  inode->i_size = block * BLOCK_SIZE;
  return -ENOSPC;
}

static void shrink_zone(struct minix2_inode *inode, size_t size) {
  block_index block, start, end;
  block_index z0, z1, z2, z3, z4;

  _page_cleanup_ struct page *page0 = buddy_alloc(BLOCK_SIZE);
  _page_cleanup_ struct page *page1 = buddy_alloc(BLOCK_SIZE);

  uint32_t *zones0 = page_address(page0), *zones1 = page_address(page1);

  start = inode->i_size / BLOCK_SIZE;
  end = size / BLOCK_SIZE;

  for (block = start; block > end; --block) {
    if (block <= 6) {
      z0 = block;
    } else if (block > 6 && block < (7 + ZONES_PER_BLOCK)) {
      z0 = 7;
    } else {
      z0 = 8;
    }

    if (!inode->i_zone[z0]) {
      continue;
    }

    switch (z0) {
      case 7:
        z1 = inode->i_zone[z0];
        z2 = block - 7;
        break;

      case 8:
        z1 = inode->i_zone[z0];
        z2 = (block - (7 + ZONES_PER_BLOCK)) / ZONES_PER_BLOCK;
        break;

      default:
        unmark_zmap(inode->i_zone[z0]);
        inode->i_zone[z0] = 0;
        continue;
    }

    block_read(z1, zones0);
    if (!zones0[z2]) {
      continue;
    }

    switch(z0) {
      case 8:
        z3 = zones0[z2];
        z4 = (block - (7 + ZONES_PER_BLOCK)) % ZONES_PER_BLOCK;
        break;

      default:
        unmark_zmap(zones0[z2]);
        zones0[z2] = 0;
        block_write(z1, zones0);

        if (!z2) {
          unmark_zmap(inode->i_zone[z0]);
          inode->i_zone[z0] = 0;
        }
        continue;
    }

    block_read(z3, zones1);
    if (!zones1[z4]) {
      continue;
    }

    unmark_zmap(zones1[z4]);
    zones1[z4] = 0;
    block_write(z3, zones1);

    if (!z4) {
      unmark_zmap(zones0[z2]);
      zones0[z2] = 0;
      block_write(z1, zones0);

      if (!z2) {
        unmark_zmap(inode->i_zone[z0]);
        inode->i_zone[z0] = 0;
      }
    }
  }

  inode->i_size = size;
}

static block_index get_block(struct inode *inode, block_index block) {
  block_index z0, z1, z2;
  struct minix2_inode minix_inode;

  _page_cleanup_ struct page *page = buddy_alloc(BLOCK_SIZE);
  uint32_t *zones = page_address(page);

  read_inode(inode->index, &minix_inode);

  if (block <= 6) {
    z0 = block;
  } else if (block > 6 && block < (7 + ZONES_PER_BLOCK)) {
    z0 = 7;
  } else {
    z0 = 8;
  }

  if (!minix_inode.i_zone[z0]) {
    return 0;
  }

  switch (z0) {
    case 7:
      z1 = minix_inode.i_zone[z0];
      z2 = block - 7;
      break;

    case 8:
      z1 = minix_inode.i_zone[z0];
      z2 = (block - (7 + ZONES_PER_BLOCK)) / ZONES_PER_BLOCK;
      break;

    default:
      return minix_inode.i_zone[z0];
  }

  block_read(z1, zones);
  if (!zones[z2]) {
    return 0;
  }

  switch(z0) {
    case 8:
      z1 = zones[z2];
      z2 = (block - (7 + ZONES_PER_BLOCK)) % ZONES_PER_BLOCK;
      break;

    default:
      return zones[z2];
  }

  block_read(z1, zones);
  if (!zones[z2]) {
    return 0;
  }

  return zones[z2];
}

static size_t calculate_block_offset(size_t start) {
  return start % BLOCK_SIZE;
}

static size_t calculate_block_copy_size(size_t start, size_t size) {
  size_t offset = calculate_block_offset(start);
  return (offset + size) > BLOCK_SIZE ? (BLOCK_SIZE - offset) : size;
}

void inode_init(void) {
  inode_cache = slab_cache_create("inode", sizeof(struct inode));
  list_init(&inodes);
}

struct inode *inode_create(uint32_t mode) {
  struct inode *inode;
  struct minix2_inode minix_inode;
  inode_index index = mark_imap();

  if (!index) {
    return NULL;
  }

  memset(&minix_inode, 0, sizeof(struct minix2_inode));
  minix_inode.i_mode = mode;
  minix_inode.i_zone[0] = mark_zmap();

  if (!minix_inode.i_zone[0]) {
    unmark_imap(index);
    return NULL;
  }

  write_inode(index, &minix_inode);

  inode = get_inode(index);
  inode->mode = mode;

  return inode;
}

int inode_destroy(struct inode *inode) {
  struct minix2_inode minix_inode;
  inode_truncate(inode, 0);

  read_inode(inode->index, &minix_inode);
  unmark_zmap(minix_inode.i_zone[0]);
  memset(&minix_inode, 0, sizeof(struct minix2_inode));
  write_inode(inode->index, &minix_inode);

  unmark_imap(inode->index);
  release_inode(inode);
  return 0;
}

int inode_truncate(struct inode *inode, size_t size) {
  int r;
  struct minix2_inode minix_inode;
  size_t tsize, tstart, toffset, tcopy;
  block_index ind_block;

  _page_cleanup_ struct page *page = buddy_alloc(BLOCK_SIZE);
  char *buf = page_address(page);

  if (size != inode->size) {
    read_inode(inode->index, &minix_inode);

    if (size > inode->size) {
      tstart = inode->size;
      tsize = size - inode->size;

      r = extend_zone(&minix_inode, size);
      if (r < 0) {
        return r;
      }

      while (tsize > 0) {
        toffset = calculate_block_offset(tstart);
        tcopy = calculate_block_copy_size(tstart, tsize);

        ind_block = get_block(inode, tstart / BLOCK_SIZE);
        if (!ind_block) {
          return -ENOSPC;
        }
        block_read(ind_block, buf);

        memset(buf + toffset, 0, tcopy);
        block_write(ind_block, buf);

        tstart += tcopy;
        tsize -= tcopy;

        SYSTEM_BUG_ON((tstart + tsize) != size);
      }
    } else {
      shrink_zone(&minix_inode, size);
    }

    inode->size = minix_inode.i_size;
    write_inode(inode->index, &minix_inode);
  }

  return 0;
}

struct inode *inode_get(inode_index index) {
  struct inode *inode;
  struct minix2_inode minix_inode;

  read_inode(index, &minix_inode);

  inode = get_inode(index);
  inode->mode = minix_inode.i_mode;
  inode->size = minix_inode.i_size;
  inode->nlinks = minix_inode.i_nlinks;

  return inode;
}

void inode_set(struct inode *inode) {
  struct minix2_inode minix_inode;
  read_inode(inode->index, &minix_inode);

  minix_inode.i_mode   = inode->mode;
  minix_inode.i_size   = inode->size;
  minix_inode.i_nlinks = inode->nlinks;

  write_inode(inode->index, &minix_inode);
}

ssize_t inode_write(struct inode *inode, size_t size, size_t start, const void *data) {
  int r, errno;
  block_index ind_zone, ind_block, ind_start, ind_end;

  const char *cur_data = data;
  size_t offset, copy, cur_size = size, cur_start = start;

  _page_cleanup_ struct page *page = buddy_alloc(BLOCK_SIZE);
  char *buf = page_address(page);

  ind_start = start / BLOCK_SIZE;
  ind_end   = (start + size) / BLOCK_SIZE;

  if (ind_end < ind_start) {
    return -1;
  }

  for (ind_zone = ind_start; ind_zone <= ind_end; ++ind_zone) {
    offset = calculate_block_offset(cur_start);
    copy = calculate_block_copy_size(cur_start, cur_size);

    if ((cur_start + copy) > inode->size) {
      r = inode_truncate(inode, cur_start + copy);
      if (r < 0) {
        errno = r;
        goto fail;
      }
    }

    ind_block = get_block(inode, ind_zone);
    if (!ind_block) {
      errno = -EINVAL;
      goto fail;
    }
    block_read(ind_block, buf);

    memcpy(buf + offset, cur_data, copy);
    block_write(ind_block, buf);

    cur_data  += copy;
    cur_start += copy;
    cur_size  -= copy;
  }

  return size;

fail:
  return (size == cur_size) ? errno : (ssize_t)(size - cur_size);
}

ssize_t inode_read(struct inode *inode, size_t size, size_t start, void *data) {
  block_index ind_zone, ind_block, ind_start, ind_end;
  size_t offset, copy, cur_size, cur_start;

  _page_cleanup_ struct page *page = buddy_alloc(BLOCK_SIZE);
  char *buf = page_address(page), *cur_data = data;

  if (inode->size <= start) {
    return 0;
  }

  if (inode->size < (start + size)) {
    size = inode->size - start;
  }

  cur_size  = size;
  cur_start = start;

  ind_start = start / BLOCK_SIZE;
  ind_end   = (start + size) / BLOCK_SIZE;

  for (ind_zone = ind_start; ind_zone <= ind_end; ++ind_zone) {
    offset = calculate_block_offset(cur_start);
    copy = calculate_block_copy_size(cur_start, cur_size);

    ind_block = get_block(inode, ind_zone);
    if (ind_block) {
      block_read(ind_block, buf);
    } else {
      memset(buf, 0, BLOCK_SIZE);
    }
    memcpy(cur_data, buf + offset, copy);

    cur_data  += copy;
    cur_start += copy;
    cur_size  -= copy;
  }

  return size;
}
