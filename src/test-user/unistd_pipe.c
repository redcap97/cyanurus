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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void seq(int fd, int last) {
  int i;
  char buf[1024];

  for (i = 1; i <= last; ++i) {
    snprintf(buf, sizeof(buf), "%d\n", i);
    TEST_ASSERT(write(fd, buf, strlen(buf)) > 0);
  }
}

void output(int fd) {
  int r;
  char buf[1024];

  while ((r = read(fd, buf, sizeof(buf) - 1))) {
    TEST_ASSERT(r > 0);
    buf[r] = '\0';
    printf("%s", buf);
  }
}

int main(void) {
  pid_t pid;
  int pipefd[2];

  TEST_START();
  TEST_ASSERT(pipe(pipefd) == 0);

  pid = fork();
  TEST_ASSERT(pid >= 0);

  if (!pid) {
    close(pipefd[0]);
    seq(pipefd[1], 4000);
    exit(0);
  }

  close(pipefd[1]);
  output(pipefd[0]);

  TEST_CHECK();
  return 0;
}
