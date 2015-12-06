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

#include <fs.c>

#include "test.h"
#include "fs.t"

#include "page.h"
#include "mmu.h"

static void generate_path(char *str, size_t size) {
  memset(str, 'a', size);
  str[0] = '/';
  str[size - 1] = '\0';
}

static void setup(void) {
  page_init();
  fs_init();

  mmu_init();
  mmu_enable();
}

TEST(test_fs_check_path) {
  char path[PATH_MAX + 100];
  generate_path(path, sizeof(path));

  TEST_ASSERT(fs_create(path, O_WRONLY|O_CREAT, 0644) == -ENAMETOOLONG);
  TEST_ASSERT(fs_unlink(path) == -ENAMETOOLONG);
  TEST_ASSERT(fs_mkdir(path, 0755) == -ENAMETOOLONG);
  TEST_ASSERT(fs_rmdir(path) == -ENAMETOOLONG);
}

TEST(test_fs_lstat64) {
  struct stat64 st;
  setup();

  TEST_ASSERT(!fs_lstat64("/sbin", &st));
  TEST_ASSERT(S_ISDIR(st.st_mode));

  TEST_ASSERT(!fs_lstat64("/sbin/init", &st));
  TEST_ASSERT(S_ISREG(st.st_mode));

  TEST_ASSERT(fs_lstat64("/sbin/command_not_found", &st) == -ENOENT);
}
