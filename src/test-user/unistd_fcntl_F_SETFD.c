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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {
  int fd;
  char fd0[1024], fd1[1024];

  char *av[] = {
    argv[0],
    NULL,
  };

  char *ep[] = {
    fd0,
    fd1,
    NULL,
  };

  (void)argc;

  TEST_START();

  {
    fd = open(argv[0], O_RDONLY);
    TEST_ASSERT(fd >= 0);
    snprintf(fd0, sizeof(fd0), "%d", fd);

    TEST_ASSERT(fcntl(fd, F_SETFD, FD_CLOEXEC) == 0);
    TEST_ASSERT(fcntl(fd, F_GETFD) == FD_CLOEXEC);
  }

  {
    fd = open(argv[0], O_RDONLY);
    TEST_ASSERT(fd >= 0);
    snprintf(fd1, sizeof(fd1), "%d", fd);

    TEST_ASSERT(fcntl(fd, F_GETFD) == 0);
  }

  execve("/sbin/unistd_fcntl_F_SETFD_new", av, ep);

  TEST_FAIL();
}
