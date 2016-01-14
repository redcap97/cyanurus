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
#include <signal.h>
#include <unistd.h>
#include <string.h>

void trap_sigsegv(int sig) {
  (void)sig;
  TEST_SUCCEED();
}

void set_signal_handler(void) {
  struct sigaction sa;
  memset(&sa, 0, sizeof(struct sigaction));

  sigemptyset(&sa.sa_mask);
  sa.sa_handler = trap_sigsegv;
  sigaction(SIGSEGV, &sa, NULL);
}

int main(void) {
  volatile char *s = NULL;

  TEST_START();

  set_signal_handler();
  *s = '!';

  TEST_FAIL();
  return 0;
}
