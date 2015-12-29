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
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <sys/uio.h>

static void check_stat(void) {
  struct stat st;

  TEST_ASSERT(stat((const char *)0xffffffffUL, &st) == -1);
  TEST_ASSERT(errno == EFAULT);

  TEST_ASSERT(stat("/", (struct stat*)0xffffffffUL) == -1);
  TEST_ASSERT(errno == EFAULT);
}

static void check_execve(void) {
  char *argv[] = {
    "ONE",
    (char*)0xffffffffUL,
    "THREE",
    NULL,
  };

  char *envp[] = {
    "PATH=/bin",
    "TERM=vt100",
    NULL,
  };

  TEST_ASSERT(execve("/sbin/command_not_found", argv, envp) == -1);
  TEST_ASSERT(errno == EFAULT);

  TEST_ASSERT(execve("/sbin/command_not_found", (char**)0xffffffffUL, envp) == -1);
  TEST_ASSERT(errno == EFAULT);
}

static void check_writev(void) {
  struct iovec iov[] = {
    { "aiueo", 5 },
    { (void*)0xffffffffUL, 5 },
  };

  TEST_ASSERT(writev(1, iov, 2) == -1);
  TEST_ASSERT(errno == EFAULT);

  TEST_ASSERT(writev(1, (struct iovec*)0xffffffffUL, 2) == -1);
  TEST_ASSERT(errno == EFAULT);
}

int main(void) {
  TEST_START();

  check_stat();
  check_execve();
  check_writev();

  TEST_SUCCEED();
  return 0;
}
