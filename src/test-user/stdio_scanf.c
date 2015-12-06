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
#include <stdio.h>
#include <string.h>

int main(void) {
  int i;
  char s[512], c;

  TEST_START();

  scanf("%c %d %s", &c, &i, s);

  TEST_ASSERT(c == 'G');
  TEST_ASSERT(i == 123);
  TEST_ASSERT(!strcmp(s, "null"));

  TEST_CHECK();
  return 0;
}
