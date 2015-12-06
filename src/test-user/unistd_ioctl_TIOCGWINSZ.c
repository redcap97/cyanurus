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
#include <sys/ioctl.h>
#include <errno.h>

#define WS_ROW 24
#define WS_COL 80

int main(void) {
  struct winsize wsz;
  int fd;
  TEST_START();

  fd = open("/a.txt", O_CREAT|O_WRONLY|O_TRUNC, 0644);
  TEST_ASSERT(ioctl(fd, TIOCGWINSZ, &wsz) == -1);
  TEST_ASSERT(errno == ENOTTY);

  TEST_ASSERT(ioctl(100, TIOCGWINSZ, &wsz) == -1);
  TEST_ASSERT(errno == EBADF);

  TEST_ASSERT(ioctl(0, TIOCGWINSZ, &wsz) == 0);
  TEST_ASSERT(wsz.ws_row == WS_ROW);
  TEST_ASSERT(wsz.ws_col == WS_COL);

  TEST_SUCCEED();
  return 0;
}
