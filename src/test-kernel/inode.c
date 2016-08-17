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

#include <inode.c>

#include "test.h"
#include "inode.t"

#include "lib/type.h"
#include "page.h"
#include "fs.h"

static void setup(void) {
  page_init();

  block_init();
  superblock_init();
  inode_init();
}

TEST(test_inode_create_0) {
  struct inode *inode;
  uint32_t mode = S_IFREG | 0755;

  setup();
  inode = inode_create(mode);

  TEST_ASSERT(inode->index != 1);
  TEST_ASSERT(inode->mode == mode);
  TEST_ASSERT(inode->nlinks == 0);
  TEST_ASSERT(inode->size == 0);
}

TEST(test_inode_create_1) {
  struct inode *inode1, *inode2;

  setup();

  inode1 = inode_create(S_IFREG | 0755);
  inode2 = inode_create(S_IFREG | 0755);

  TEST_ASSERT(inode1->index != inode2->index);
}

TEST(test_inode_create_2) {
  struct inode *inode;
  struct minix2_inode minix_inode;
  uint32_t mode = S_IFREG | 0755;

  setup();

  inode = inode_create(mode);
  read_inode(inode->index, &minix_inode);

  TEST_ASSERT(minix_inode.i_mode == mode);
  TEST_ASSERT(minix_inode.i_nlinks == 0);
  TEST_ASSERT(minix_inode.i_size == 0);

  TEST_ASSERT(minix_inode.i_zone[0] != 0);
  TEST_ASSERT(minix_inode.i_zone[1] == 0);
}

TEST(test_inode_create_3) {
  struct inode *inode;

  setup();
  TEST_ASSERT(list_length(&inodes) == 0);

  inode = inode_create(S_IFREG | 0755);
  TEST_ASSERT(inode == inode_get(inode->index));
  TEST_ASSERT(list_length(&inodes) == 1);

  inode = inode_create(S_IFREG | 0755);
  TEST_ASSERT(inode == inode_get(inode->index));
  TEST_ASSERT(list_length(&inodes) == 2);
}

TEST(test_inode_destroy_0) {
  struct inode *inode;

  setup();
  TEST_ASSERT(list_length(&inodes) == 0);

  inode = inode_create(S_IFREG | 0755);
  TEST_ASSERT(list_length(&inodes) == 1);

  inode_destroy(inode);
  TEST_ASSERT(list_length(&inodes) == 0);
}

TEST(test_inode_destroy_1) {
  struct inode *inode;
  struct minix2_inode minix_inode;
  inode_index index;

  setup();

  inode = inode_create(S_IFREG | 0755);
  index = inode->index;

  read_inode(index, &minix_inode);
  TEST_ASSERT(minix_inode.i_mode != 0);

  inode_destroy(inode);
  read_inode(index, &minix_inode);
  TEST_ASSERT(minix_inode.i_mode == 0);
}
