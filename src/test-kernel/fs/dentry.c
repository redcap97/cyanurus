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

#include <fs/dentry.c>

#include "test.h"
#include "fs/dentry.t"

#include "fs/superblock.h"
#include "page.h"

static void setup(void) {
  page_init();

  fs_block_init();
  fs_superblock_init();
  fs_inode_init();
  fs_dentry_init();
}

static void assert_parent_child(const struct dentry *parent, const struct dentry *child) {
  struct dentry *dentry;
  TEST_ASSERT(child->parent == parent);

  list_foreach(dentry, &parent->children, sibling) {
    if (dentry == child) {
      return;
    }
  }

  TEST_FAIL();
}

TEST(test_fs_dentry_init) {
  setup();

  TEST_ASSERT(root_dentry == root_dentry->parent);
  TEST_ASSERT(root_dentry->inode->index == 1);
}

TEST(test_fs_dentry_lookup_0) {
  setup();
  TEST_ASSERT(fs_dentry_lookup("/") != NULL);

  TEST_ASSERT(fs_dentry_lookup("/var") == NULL);
  TEST_ASSERT(fs_dentry_lookup("/var/lib") == NULL);
}

TEST(test_fs_dentry_lookup_1) {
  struct dentry *dentry, *parent_dentry;

  setup();

  dentry = fs_dentry_lookup("/sbin/init");

  TEST_ASSERT(dentry != NULL);
  TEST_ASSERT(dentry->nentries == 0);
  TEST_ASSERT(dentry->inode != NULL);
  TEST_ASSERT(dentry->inode->nlinks == 1);

  parent_dentry = fs_dentry_lookup("/sbin");
  assert_parent_child(parent_dentry, dentry);

  TEST_ASSERT(parent_dentry != NULL);
  TEST_ASSERT(parent_dentry->nentries == 3);
  TEST_ASSERT(parent_dentry->inode != NULL);
  TEST_ASSERT(parent_dentry->inode->nlinks == 2);
}

TEST(test_fs_dentry_link) {
  struct inode *inode;

  setup();
  fs_dentry_lookup("/");

  inode = fs_inode_create(S_IFREG | 0755);
  TEST_ASSERT(inode != NULL);

  TEST_ASSERT(inode->nlinks == 0);
  TEST_ASSERT(root_dentry->nentries == 2);

  TEST_ASSERT(!fs_dentry_link(root_dentry, "new", inode));

  TEST_ASSERT(inode->nlinks == 1);
  TEST_ASSERT(root_dentry->nentries == 3);

  assert_parent_child(root_dentry, fs_dentry_lookup("/new"));
}

TEST(test_fs_dentry_unlink) {
  struct inode *inode;
  struct dentry *dentry, *parent_dentry;

  setup();

  dentry = fs_dentry_lookup("/sbin/init");
  TEST_ASSERT(dentry != NULL);

  parent_dentry = dentry->parent;
  inode = dentry->inode;

  TEST_ASSERT(inode->nlinks == 1);
  TEST_ASSERT(parent_dentry->nentries == 3);

  TEST_ASSERT(!fs_dentry_unlink(dentry));

  TEST_ASSERT(inode->nlinks == 0);
  TEST_ASSERT(parent_dentry->nentries == 2);

  list_foreach(dentry, &parent_dentry->children, sibling) {
    if (!strncmp(dentry->name, "init", NAME_MAX)) {
      TEST_FAIL();
    }
  }
}
