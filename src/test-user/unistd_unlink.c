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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#define DATA_FILE "/tmp/data.txt"

int main(void) {
  int fd;
  struct stat st;
  TEST_START();

  TEST_ASSERT(unlink(DATA_FILE) == -1);
  TEST_ASSERT(errno == ENOENT);

  fd = open(DATA_FILE, O_WRONLY | O_CREAT, 0644);
  TEST_ASSERT(fd >= 0);
  close(fd);

  TEST_ASSERT(stat(DATA_FILE, &st) == 0);
  TEST_ASSERT(unlink(DATA_FILE) == 0);

  TEST_ASSERT(stat(DATA_FILE, &st) == -1);
  TEST_ASSERT(errno == ENOENT);

  TEST_SUCCEED();
  return 0;
}
