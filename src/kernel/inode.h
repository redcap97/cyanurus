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

#ifndef _CYANURUS_FS_INODE_H_
#define _CYANURUS_FS_INODE_H_

#include "lib/type.h"
#include "lib/list.h"
#include "block.h"

#define S_ISREG(m)  (m & S_IFREG)
#define S_ISDIR(m)  (m & S_IFDIR)
#define S_ISCHR(m)  (m & S_IFCHR)
#define S_ISBLK(m)  (m & S_IFBLK)
#define S_ISFIFO(m) (m & S_IFIFO)
#define S_ISLNK(m)  (m & S_IFLNK)
#define S_ISSOCK(m) (m & S_IFSOCK)

#define S_IFMT   0170000
#define S_IFSOCK 0140000
#define S_IFLNK  0120000
#define S_IFREG  0100000
#define S_IFBLK  0060000
#define S_IFDIR  0040000
#define S_IFCHR  0020000
#define S_IFIFO  0010000
#define S_ISUID  0004000
#define S_ISGID  0002000
#define S_ISVTX  0001000
#define S_IRWXU  00700
#define S_IRUSR  00400
#define S_IWUSR  00200
#define S_IXUSR  00100
#define S_IRWXG  00070
#define S_IRGRP  00040
#define S_IWGRP  00020
#define S_IXGRP  00010
#define S_IRWXO  00007
#define S_IROTH  00004
#define S_IWOTH  00002
#define S_IXOTH  00001

typedef uint32_t inode_index;

struct inode {
  inode_index index;
  struct list next;
  uint32_t mode;
  uint32_t nlinks;
  size_t size;
};

void inode_init(void);
struct inode *inode_create(uint32_t mode);
int inode_destroy(struct inode *inode);
int inode_truncate(struct inode *inode, size_t size);
struct inode *inode_get(inode_index index);
void inode_set(struct inode *inode);
ssize_t inode_write(struct inode *inode, size_t size, size_t start, const void *data);
ssize_t inode_read(struct inode *inode, size_t size, size_t start, void *data);

#endif
