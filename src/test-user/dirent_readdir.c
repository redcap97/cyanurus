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

#include <test.h>
#include <dirent.h>
#include <string.h>

struct entry {
  char *name;
  size_t count;
};

void test_entries(const char *path, struct entry *entries, size_t len) {
  size_t i;
  DIR *dirp;
  struct dirent *dirent;

  TEST_ASSERT((dirp = opendir(path)) != NULL);

  while ((dirent = readdir(dirp))) {
    for (i = 0; i < len; ++i) {
      if (!strcmp(dirent->d_name, entries[i].name)) {
        entries[i].count++;
        break;
      }
    }

    if (i == len) {
      TEST_FAIL();
      return;
    }
  }

  for (i = 0; i < len; ++i) {
    TEST_ASSERT(entries[i].count == 1);
  }

  closedir(dirp);
}

int main(void) {
  struct entry root_entries[] = {
    {".", 0}, {"..", 0}, {"sbin", 0}
  };

  struct entry sbin_entries[] = {
    {".", 0}, {"..", 0}, {"dirent_readdir", 0}
  };

  TEST_START();

  test_entries("/", root_entries, 3);
  test_entries("/sbin", sbin_entries, 3);

  TEST_SUCCEED();
  return 0;
}
