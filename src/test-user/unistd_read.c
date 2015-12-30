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
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#include "alice_text.h"

static void check_read_directory(void) {
  int fd;
  char buf[32];

  fd = open("/usr/share", O_RDONLY);
  TEST_ASSERT(fd >= 0);

  TEST_ASSERT(read(fd, buf, sizeof(buf)) == -1);
  TEST_ASSERT(errno == EISDIR);

  TEST_ASSERT(!close(fd));
}

static void check_read_file(void) {
  int fd;
  ssize_t size, total = 0;
  char buf[32], text[4096] = "";

  fd = open("/usr/share/alice.txt", O_RDONLY);
  TEST_ASSERT(fd >= 0);

  while (1) {
    size = read(fd, buf, sizeof(buf) - 1);
    TEST_ASSERT(size >= 0);

    if (!size) {
      break;
    }
    buf[size] = '\0';
    total += size;

    TEST_ASSERT((total + 1) <= (ssize_t)sizeof(text));
    strcat(text, buf);
  }

  TEST_ASSERT(!strcmp(ALICE_TEXT, text));
  TEST_ASSERT(!close(fd));
}

int main(void) {
  TEST_START();

  check_read_directory();
  check_read_file();

  TEST_SUCCEED();
  return 0;
}
