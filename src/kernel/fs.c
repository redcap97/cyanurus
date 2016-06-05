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

#include "fs.h"
#include "lib/string.h"
#include "lib/libgen.h"
#include "lib/errno.h"
#include "fs/block.h"
#include "fs/superblock.h"
#include "fs/inode.h"
#include "fs/dentry.h"
#include "buddy.h"

static void remove_dentry_and_children(struct dentry *dentry) {
  struct dentry *child, *temp;

  list_foreach_safe(child, temp, &dentry->children, sibling) {
    dentry_unlink(child);
  }

  dentry_unlink(dentry);
}

void fs_init(void) {
  block_init();
  superblock_init();
  fs_inode_init();
  dentry_init();
}

int fs_create(const char *path, int flags, mode_t mode) {
  struct dentry *dentry = NULL;
  struct inode *inode = NULL;
  int errno = -EINVAL;

  _page_cleanup_ struct page *page = buddy_alloc(PATH_MAX);
  char *buf = page_address(page);

  if (strnlen(path, PATH_MAX) == PATH_MAX) {
    return -ENAMETOOLONG;
  }

  if ((dentry = dentry_lookup(path))) {
    return (flags & O_EXCL) ? -EEXIST : 0;
  }

  strcpy(buf, path);
  if (!(dentry = dentry_lookup(dirname(buf)))) {
    return -EEXIST;
  }

  if (!(inode = fs_inode_create((mode & 0777) | S_IFREG))) {
    errno = -ENOSPC;
    goto fail;
  }

  strcpy(buf, path);
  if ((errno = dentry_link(dentry, basename(buf), inode)) < 0) {
    goto fail;
  }

  return 0;

fail:
  if (inode) {
    fs_inode_destroy(inode);
  }
  return errno;
}

int fs_unlink(const char *path) {
  struct dentry *dentry;
  struct inode *inode;
  ssize_t r;

  if (strnlen(path, PATH_MAX) == PATH_MAX) {
    return -ENAMETOOLONG;
  }

  if (!(dentry = dentry_lookup(path))) {
    return -ENOENT;
  }

  inode = dentry->inode;
  if (S_ISDIR(inode->mode)) {
    return -EISDIR;
  }

  if ((r = dentry_unlink(dentry)) < 0) {
    return r;
  }

  if (inode->nlinks == 0) {
    fs_inode_destroy(inode);
  }

  return 0;
}

int fs_mkdir(const char *path, mode_t mode) {
  struct dentry *dentry = NULL, *parent_dentry = NULL;
  struct inode *inode = NULL;
  int errno = -EINVAL;

  _page_cleanup_ struct page *page = buddy_alloc(PATH_MAX);
  char *buf = page_address(page);

  if (strnlen(path, PATH_MAX) == PATH_MAX) {
    return -ENAMETOOLONG;
  }

  if (dentry_lookup(path)) {
    return -EEXIST;
  }

  strcpy(buf, path);
  if (!(parent_dentry = dentry_lookup(dirname(buf)))) {
    return -ENOENT;
  }

  strcpy(buf, path);
  if (!(inode = fs_inode_create((mode & 0777) | S_IFDIR))) {
    errno = -ENOSPC;
    goto fail;
  }

  if ((errno = dentry_link(parent_dentry, basename(buf), inode)) < 0) {
    goto fail;
  }

  if (!(dentry = dentry_lookup(path))) {
    goto fail;
  }

  if ((errno = dentry_link(dentry, ".", inode)) < 0) {
    goto fail;
  }

  if ((errno = dentry_link(dentry, "..", dentry->parent->inode)) < 0) {
    goto fail;
  }

  return 0;

fail:
  if (dentry) {
    remove_dentry_and_children(dentry);
  }

  if (inode) {
    fs_inode_destroy(inode);
  }

  return errno;
}

int fs_rmdir(const char *path) {
  struct dentry *dentry, *child;
  struct inode *inode;

  if (strnlen(path, PATH_MAX) == PATH_MAX) {
    return -ENAMETOOLONG;
  }

  if (!(dentry = dentry_lookup(path))) {
    return -ENOENT;
  }

  if (!(S_ISDIR(dentry->inode->mode))) {
    return -ENOTDIR;
  }

  if (dentry->nentries != 2) {
    return -ENOTEMPTY;
  }

  list_foreach(child, &dentry->children, sibling) {
    if (strncmp(".",  child->name, NAME_MAX) &&
        strncmp("..", child->name, NAME_MAX))
    {
      return -EINVAL;
    }
  }

  inode = dentry->inode;
  remove_dentry_and_children(dentry);
  fs_inode_destroy(inode);

  return 0;
}

int fs_lstat64(const char *path, struct stat64 *buf) {
  struct dentry *dentry;

  if (!(dentry = dentry_lookup(path))) {
    return -ENOENT;
  }

  memset(buf, 0, sizeof(struct stat64));
  buf->st_ino = dentry->inode->index;
  buf->st_mode = dentry->inode->mode;
  buf->st_size = dentry->inode->size;
  buf->st_nlink = dentry->inode->nlinks;
  buf->st_blocks = buf->st_size / BLOCK_SIZE;
  buf->st_blksize = BLOCK_SIZE;

  return 0;
}

int fs_fstat64(const struct file *file, struct stat64 *buf) {
  struct dentry *dentry;

  switch(file->type) {
    case FF_TTY:
    case FF_PIPE:
      memset(buf, 0, sizeof(struct stat64));
      buf->st_mode = (S_IFCHR|S_IRUSR|S_IWUSR);
      buf->st_size = 0;
      buf->st_nlink = 1;
      buf->st_blocks = 0;
      buf->st_blksize = BLOCK_SIZE;
      return 0;

    case FF_INODE:
      dentry = file->dentry;
      memset(buf, 0, sizeof(struct stat64));
      buf->st_ino = dentry->inode->index;
      buf->st_mode = dentry->inode->mode;
      buf->st_size = dentry->inode->size;
      buf->st_nlink = dentry->inode->nlinks;
      buf->st_blocks = buf->st_size / BLOCK_SIZE;
      buf->st_blksize = BLOCK_SIZE;
      return 0;

    default:
      return -EINVAL;
  }
}
