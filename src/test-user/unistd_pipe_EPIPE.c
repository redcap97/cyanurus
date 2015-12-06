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
#include <errno.h>

int main(void) {
  int pipefd[2];
  char data[1024 * 8];

  TEST_START();
  TEST_ASSERT(pipe(pipefd) == 0);

  close(pipefd[0]);
  TEST_ASSERT(write(pipefd[1], data, sizeof(data)) == -1);
  TEST_ASSERT(errno == EPIPE);

  TEST_SUCCEED();
  return 0;
}
