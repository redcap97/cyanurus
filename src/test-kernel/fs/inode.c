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

#include <fs/inode.c>

#include "test.h"
#include "fs/inode.t"

#include "lib/type.h"
#include "page.h"
#include "fs.h"

static void setup(void) {
  page_init();

  fs_block_init();
  fs_superblock_init();
  fs_inode_init();
}

TEST(test_fs_inode_create_0) {
  struct inode *inode;
  uint32_t mode = S_IFREG | 0755;

  setup();
  inode = fs_inode_create(mode);

  TEST_ASSERT(inode->index != 1);
  TEST_ASSERT(inode->mode == mode);
  TEST_ASSERT(inode->nlinks == 0);
  TEST_ASSERT(inode->size == 0);
}

TEST(test_fs_inode_create_1) {
  struct inode *inode1, *inode2;

  setup();

  inode1 = fs_inode_create(S_IFREG | 0755);
  inode2 = fs_inode_create(S_IFREG | 0755);

  TEST_ASSERT(inode1->index != inode2->index);
}

TEST(test_fs_inode_create_2) {
  struct inode *inode;
  struct minix2_inode minix_inode;
  uint32_t mode = S_IFREG | 0755;

  setup();

  inode = fs_inode_create(mode);
  read_inode(inode->index, &minix_inode);

  TEST_ASSERT(minix_inode.i_mode == mode);
  TEST_ASSERT(minix_inode.i_nlinks == 0);
  TEST_ASSERT(minix_inode.i_size == 0);

  TEST_ASSERT(minix_inode.i_zone[0] != 0);
  TEST_ASSERT(minix_inode.i_zone[1] == 0);
}

TEST(test_fs_inode_create_3) {
  struct inode *inode;

  setup();
  TEST_ASSERT(list_length(&inodes) == 0);

  inode = fs_inode_create(S_IFREG | 0755);
  TEST_ASSERT(inode == fs_inode_get(inode->index));
  TEST_ASSERT(list_length(&inodes) == 1);

  inode = fs_inode_create(S_IFREG | 0755);
  TEST_ASSERT(inode == fs_inode_get(inode->index));
  TEST_ASSERT(list_length(&inodes) == 2);
}

TEST(test_fs_inode_destroy) {
  struct inode *inode;

  setup();
  TEST_ASSERT(list_length(&inodes) == 0);

  inode = fs_inode_create(S_IFREG | 0755);
  TEST_ASSERT(list_length(&inodes) == 1);

  fs_inode_destroy(inode);
  TEST_ASSERT(list_length(&inodes) == 0);
}
