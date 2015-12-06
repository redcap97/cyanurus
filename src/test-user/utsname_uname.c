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
#include <string.h>
#include <sys/utsname.h>

int main(void) {
  struct utsname uts;
  TEST_START();

  TEST_ASSERT(uname(&uts) == 0);
  TEST_ASSERT(strcmp(uts.sysname, "Cyanurus") == 0);

  TEST_SUCCEED();
  return 0;
}
