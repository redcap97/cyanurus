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
#include <termios.h>
#include <errno.h>
#include <string.h>

#define OFLAG (OPOST|ONLCR)
#define IFLAG (ICRNL|IXON|IXANY|IMAXBEL)
#define CFLAG (CREAD|CS8|B9600)
#define LFLAG (ISIG|ICANON|IEXTEN|ECHO|ECHOE|ECHOCTL|ECHOKE)

int main(void) {
  struct termios tio;
  int fd;
  TEST_START();
  memset(&tio, 0, sizeof(struct termios));

  fd = open("/a.txt", O_CREAT|O_WRONLY|O_TRUNC, 0644);
  TEST_ASSERT(tcgetattr(fd, &tio) == -1);
  TEST_ASSERT(errno == ENOTTY);

  TEST_ASSERT(tcgetattr(100, &tio) == -1);
  TEST_ASSERT(errno == EBADF);

  TEST_ASSERT(tcgetattr(0, &tio) == 0);
  TEST_ASSERT(tio.c_oflag == OFLAG);
  TEST_ASSERT(tio.c_iflag == IFLAG);
  TEST_ASSERT(tio.c_cflag == CFLAG);
  TEST_ASSERT(tio.c_lflag == LFLAG);
  TEST_ASSERT(tio.c_cc[VTIME] == 0);
  TEST_ASSERT(tio.c_cc[VMIN] == 1);

  TEST_SUCCEED();
  return 0;
}
