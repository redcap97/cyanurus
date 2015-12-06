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

#define DATA_FILE "/tmp/data.txt"

static void append(const char *str) {
  int fd;

  fd = open(DATA_FILE, O_APPEND | O_WRONLY | O_CREAT, 0644);
  TEST_ASSERT(fd >= 0);

  TEST_ASSERT(write(fd, str, strlen(str)) > 0);
  close(fd);
}

static void check(void) {
  int fd;
  ssize_t r;
  char buf[1024];

  fd = open(DATA_FILE, O_RDONLY);
  TEST_ASSERT(fd >= 0);

  r = read(fd, buf, sizeof(buf));

  TEST_ASSERT(r > 0);
  buf[r] = '\0';

  TEST_ASSERT(strcmp(buf, "'Cos I'm guilty\n") == 0);
}

int main(void) {
  TEST_START();

  append("'Cos");
  append(" ");
  append("I'm");
  append(" ");
  append("guilty");
  append("\n");

  check();

  TEST_SUCCEED();
  return 0;
}
