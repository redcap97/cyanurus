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

#include "unistd_execve.h"

int main(int argc, char *argv[]) {
  TEST_START();

  TEST_ASSERT(argc == 1);
  TEST_ASSERT(strlen(argv[0]) > 0);
  TEST_ASSERT(execve("/sbin/command_not_found", av, ep) < 0);

  TEST_SUCCEED();
}
