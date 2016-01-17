/*
Copyright 2016 Akira Midorikawa

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
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define FILENAME "/tmp/lseek"

static void for_tty(void) {
  TEST_ASSERT(lseek(0, 0, SEEK_SET) == -1);
  TEST_ASSERT(errno == ESPIPE);
}

static void for_pipe(void) {
  int pipefd[2];

  TEST_ASSERT(pipe(pipefd) == 0);

  TEST_ASSERT(lseek(pipefd[0], 0, SEEK_SET) == -1);
  TEST_ASSERT(errno == ESPIPE);
}

static void for_file_write(void) {
  int fd;

  fd = open(FILENAME, O_CREAT|O_WRONLY|O_TRUNC, 0644);
  TEST_ASSERT(fd >= 0);

  TEST_ASSERT(write(fd, "XXXXX", 5) == 5);

  TEST_ASSERT(lseek(fd, 0, SEEK_SET) == 0);
  TEST_ASSERT(write(fd, "O", 1) == 1);

  TEST_ASSERT(lseek(fd, 1, SEEK_CUR) == 2);
  TEST_ASSERT(write(fd, "O", 1) == 1);

  TEST_ASSERT(lseek(fd, -1, SEEK_END) == 4);
  TEST_ASSERT(write(fd, "O", 1) == 1);

  close(fd);
}

static void for_file_read(void) {
  int fd;
  char buf[1024];

  fd = open(FILENAME, O_RDONLY);
  TEST_ASSERT(fd >= 0);

  TEST_ASSERT(read(fd, buf, sizeof(buf)) == 5);
  buf[5] = '\0';

  TEST_ASSERT(!strcmp(buf, "OXOXO"));

  close(fd);
}

int main(void) {
  TEST_START();

  for_tty();
  for_pipe();

  for_file_write();
  for_file_read();

  TEST_SUCCEED();
  return 0;
}
