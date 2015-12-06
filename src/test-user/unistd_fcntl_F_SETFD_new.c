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
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>

extern char **environ;

int main(int argc, char *argv[]) {
  int fd;
  char *fd0 = environ[0], *fd1 = environ[1];

  (void)argc;
  (void)argv;

  {
    TEST_ASSERT(fd0 != NULL);
    TEST_ASSERT(strlen(fd0) > 0);

    fd = strtol(fd0, NULL, 10);
    TEST_ASSERT(errno != ERANGE && errno != EINVAL);

    TEST_ASSERT(fcntl(fd, F_GETFD) < 0);
    TEST_ASSERT(errno == EBADF);
  }

  {
    TEST_ASSERT(fd1 != NULL);
    TEST_ASSERT(strlen(fd1) > 0);

    fd = strtol(fd1, NULL, 10);
    TEST_ASSERT(errno != ERANGE && errno != EINVAL);

    TEST_ASSERT(fcntl(fd, F_GETFD) == 0);
  }

  TEST_SUCCEED();
}
