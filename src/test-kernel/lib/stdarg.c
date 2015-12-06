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

#include <lib/stdarg.c>

#include "test.h"
#include "lib/stdarg.t"

#include "lib/string.h"
#include "logger.h"

TEST(test_stdarg_snprintf_0) {
  char buf[1024];

  TEST_ASSERT(snprintf(buf, sizeof(buf), "aiueo%%") > 0);
  TEST_ASSERT(strcmp(buf, "aiueo%") == 0);

  TEST_ASSERT(snprintf(buf, sizeof(buf), "char: '%c' '%1c' '%4c'", 'a', 'b', 'c') > 0);
  TEST_ASSERT(strcmp(buf, "char: 'a' 'b' '   c'") == 0);

  TEST_ASSERT(snprintf(buf, sizeof(buf), "str: '%s' '%1s' '%4s'", "ka", "ki", "ku") > 0);
  TEST_ASSERT(strcmp(buf, "str: 'ka' 'ki' '  ku'") == 0);

  TEST_ASSERT(snprintf(buf, sizeof(buf), "hex: '%x' '%6x' '%06x'", 0xffffffff, 0xb2, 0xc3) > 0);
  TEST_ASSERT(strcmp(buf, "hex: 'ffffffff' '    b2' '0000c3'") == 0);

  TEST_ASSERT(snprintf(buf, sizeof(buf), "signed dec: '%d' '%4d' '%04d'", 10, -1, -2) > 0);
  TEST_ASSERT(strcmp(buf, "signed dec: '10' '  -1' '-002'") == 0);

  TEST_ASSERT(snprintf(buf, sizeof(buf), "unsigned dec: '%u' '%4u' '%04u'", 0xffffffff, 1, 2) > 0);
  TEST_ASSERT(strcmp(buf, "unsigned dec: '4294967295' '   1' '0002'") == 0);
}

TEST(test_stdarg_snprintf_1) {
  size_t i;
  char buf[1024];

  TEST_ASSERT(snprintf(buf, (size_t)INT_MAX + 1, "aiueo") == EOF);

  TEST_ASSERT(snprintf(buf, sizeof(buf), "aiueo") == 5);
  TEST_ASSERT(strlen(buf) == 5);

  for (i = 6; i > 0; --i) {
    TEST_ASSERT(snprintf(buf, i, "aiueo") == 5);
    TEST_ASSERT(strlen(buf) == (i - 1));
  }

  TEST_ASSERT(snprintf(buf, 0, "aiueo") == 5);
}
