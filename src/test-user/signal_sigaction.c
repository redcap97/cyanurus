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
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

void trap_sigint(int sig) {
  sigset_t set;

  sigprocmask(0, NULL, &set);
  TEST_ASSERT(sigismember(&set, sig));

  _exit(97);
}

int main(void) {
  int st;
  pid_t pid;
  struct sigaction sa;

  TEST_START();

  pid = fork();
  TEST_ASSERT(pid >= 0);

  if (!pid) {
    memset(&sa, 0, sizeof(struct sigaction));
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = trap_sigint;

    TEST_ASSERT(sigaction(SIGINT, &sa, NULL) == 0);
    kill(getpid(), SIGINT);

    while(1);
  }
  wait(&st);

  TEST_ASSERT(WEXITSTATUS(st) == 97);
  TEST_SUCCEED();
  return 0;
}
