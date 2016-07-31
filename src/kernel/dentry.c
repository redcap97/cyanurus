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

#include "dentry.h"
#include "lib/string.h"
#include "slab.h"
#include "logger.h"
#include "uart.h"
#include "buddy.h"

#define DIRENT_SIZE sizeof(struct minix3_dirent)

static struct slab_cache *dentry_cache;
static struct dentry *root_dentry;

static struct dentry *alloc_dentry(struct dentry *parent, struct inode *inode, const char *name) {
  struct dentry *new = slab_cache_alloc(dentry_cache);
  memset(new, 0, sizeof(struct dentry));

  if (parent) {
    new->parent = parent;
  } else {
    new->parent = new;
  }
  new->inode = inode;

  strncpy(new->name, name, NAME_MAX);
  list_init(&new->children);

  return new;
}

static int read_children(struct dentry *dentry) {
  ssize_t rs, i;
  size_t offset = 0;
  struct minix3_dirent *dirent;
  struct dentry *child_dentry, *temp_dentry;
  struct list children;

  _page_cleanup_ struct page *page = buddy_alloc(BLOCK_SIZE);
  struct minix3_dirent *dirents = page_address(page);

  list_init(&children);

  if (dentry->flags & DF_FLAGS_LOAD) {
    return 0;
  }

  if (!S_ISDIR(dentry->inode->mode)) {
    dentry->flags |= DF_FLAGS_LOAD;
    return 0;
  }

  while (1) {
    if ((rs = inode_read(dentry->inode, BLOCK_SIZE, offset, dirents)) < 0) {
      goto fail;
    }

    if (rs & (DIRENT_SIZE - 1)) {
      goto fail;
    }

    if (rs == 0) {
      break;
    }

    for (i = 0; i < (rs / (ssize_t)DIRENT_SIZE); ++i) {
      dirent = &dirents[i];

      if (dirent->d_ino) {
        child_dentry = alloc_dentry(dentry, inode_get(dirent->d_ino), dirent->d_name);
        list_add(&children, &child_dentry->sibling);
      }
    }

    offset += rs;
  }

  dentry->nentries += list_length(&children);
  list_concat(&dentry->children, &children);

  dentry->flags |= DF_FLAGS_LOAD;
  return 0;

fail:
  list_foreach_safe(child_dentry, temp_dentry, &children, sibling) {
    slab_cache_free(dentry_cache, child_dentry);
  }
  return -1;
}

static int write_child(struct dentry *dentry) {
  struct minix3_dirent dirent;
  struct dentry *parent = dentry->parent;

  dirent.d_ino = dentry->inode->index;
  memset(dirent.d_name, 0, NAME_MAX);
  strcpy(dirent.d_name, dentry->name);

  if (inode_write(parent->inode, DIRENT_SIZE, parent->inode->size, &dirent) < 0) {
    return -1;
  }

  return 0;
}

static int remove_child(struct dentry *dentry, const char *name) {
  ssize_t rs, i;
  size_t offset = 0;
  struct minix3_dirent *dirent;

  _page_cleanup_ struct page *page = buddy_alloc(BLOCK_SIZE);
  struct minix3_dirent *dirents = page_address(page);

  while (1) {
    if ((rs = inode_read(dentry->inode, BLOCK_SIZE, offset, dirents)) < 0) {
      return -1;
    }

    if (rs == 0 || (rs & (DIRENT_SIZE - 1))) {
      return -1;
    }

    for (i = 0; i < (rs / (ssize_t)DIRENT_SIZE); ++i) {
      dirent = &dirents[i];

      if (dirent->d_ino && !strncmp(dirent->d_name, name, NAME_MAX)) {
        dirent->d_ino = 0;

        if (inode_write(dentry->inode, DIRENT_SIZE, offset + (i * DIRENT_SIZE), dirent) != DIRENT_SIZE) {
          return -1;
        }

        return 0;
      }
    }

    offset += rs;
  }
}

static struct dentry *lookup_dentry(const char *path) {
  char name[NAME_MAX];
  struct dentry *current_dentry = root_dentry, *child_dentry, *dentry;
  const char *p = path, *pn;

  if (*p != '/') {
    return NULL;
  }

  while (*p++) {
    read_children(current_dentry);

    pn = strchrnul(p, '/');
    if (pn == p) {
      continue;
    }

    if ((pn - p) > NAME_MAX) {
      return NULL;
    }

    child_dentry = NULL;
    memset(name, 0, NAME_MAX);
    strncpy(name, p, pn - p);

    list_foreach(dentry, &current_dentry->children, sibling) {
      if (!strncmp(dentry->name, name, NAME_MAX)) {
        child_dentry = dentry;
        break;
      }
    }

    if (!child_dentry) {
      return NULL;
    }

    current_dentry = child_dentry;
    p = pn;
  }

  read_children(current_dentry);
  return current_dentry;
}

static int create_dentry(struct dentry *dentry, const char *name, struct inode *inode) {
  struct dentry *child, *new;

  if (strnlen(name, NAME_MAX + 1) > NAME_MAX) {
    return -1;
  }

  read_children(dentry);
  list_foreach(child, &dentry->children, sibling) {
    if (!strncmp(child->name, name, NAME_MAX)) {
      return -1;
    }
  }

  new = alloc_dentry(dentry, inode, name);
  list_add(&dentry->children, &new->sibling);
  write_child(new);

  dentry->nentries++;
  new->inode->nlinks++;
  inode_set(new->inode);

  return 0;
}

static int destroy_dentry(struct dentry *dentry) {
  struct dentry *parent = dentry->parent;

  if (remove_child(dentry->parent, dentry->name) < 0) {
    return -1;
  }

  parent->nentries--;
  dentry->inode->nlinks--;
  inode_set(dentry->inode);

  list_remove(&dentry->sibling);
  slab_cache_free(dentry_cache, dentry);

  return 0;
}

void dentry_init(void) {
  dentry_cache = slab_cache_create("dentry", sizeof(struct dentry));
  root_dentry = alloc_dentry(NULL, inode_get(1), "");
}

struct dentry *dentry_lookup(const char *path) {
  if (strnlen(path, PATH_MAX) == PATH_MAX) {
    return NULL;
  }
  return lookup_dentry(path);
}

int dentry_link(struct dentry *dentry, const char *path, struct inode *inode) {
  return create_dentry(dentry, path, inode);
}

int dentry_unlink(struct dentry *dentry) {
  return destroy_dentry(dentry);
}
