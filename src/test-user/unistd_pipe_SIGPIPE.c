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
#include <string.h>
#include <stdlib.h>
#include <signal.h>

void trap_sigpipe(int sig) {
  TEST_ASSERT(sig == SIGPIPE);
  TEST_SUCCEED();
}

void set_signal_handler(void) {
  struct sigaction sa;
  memset(&sa, 0, sizeof(struct sigaction));

  sigemptyset(&sa.sa_mask);
  sa.sa_handler = trap_sigpipe;
  TEST_ASSERT(sigaction(SIGPIPE, &sa, NULL) == 0);
}

int main(void) {
  pid_t pid;
  int pipefd[2];
  char data[1024 * 8];

  TEST_START();
  TEST_ASSERT(pipe(pipefd) == 0);

  pid = fork();
  TEST_ASSERT(pid >= 0);

  if (!pid) {
    TEST_ASSERT(read(pipefd[0], data, 1) == 1);
    exit(0);
  }

  set_signal_handler();
  close(pipefd[0]);
  write(pipefd[1], data, sizeof(data));

  TEST_FAIL();
  return 0;
}
