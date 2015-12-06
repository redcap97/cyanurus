/*
Copyright 2015 Akira Midorikawa

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
#include <string.h>
#include <fcntl.h>
#include <errno.h>

static void check_O_RDONLY(void) {
  TEST_ASSERT(open("/", O_RDONLY) >= 0);
}

static void check_O_WRONLY(void) {
  TEST_ASSERT(open("/", O_WRONLY) == -1);
  TEST_ASSERT(errno == EISDIR);
}

static void check_O_RDWR(void) {
  TEST_ASSERT(open("/", O_RDWR) == -1);
  TEST_ASSERT(errno == EISDIR);
}

int main(void) {
  TEST_START();

  check_O_RDONLY();
  check_O_WRONLY();
  check_O_RDWR();

  TEST_SUCCEED();
  return 0;
}
