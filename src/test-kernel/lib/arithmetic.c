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

#include <lib/arithmetic.c>

#include "test.h"
#include "lib/arithmetic.t"

TEST(test_add_overflow_unsigned_long) {
  TEST_ASSERT(!add_overflow_unsigned_long(0x00000000UL, 0x00000000UL));
  TEST_ASSERT(!add_overflow_unsigned_long(0xffff0000UL, 0x0000ffffUL));
  TEST_ASSERT(!add_overflow_unsigned_long(0xfffffffeUL, 0x00000001UL));

  TEST_ASSERT(add_overflow_unsigned_long(0xffffffffUL, 0x00000001UL));
  TEST_ASSERT(add_overflow_unsigned_long(0xffff0000UL, 0x0001ffffUL));
  TEST_ASSERT(add_overflow_unsigned_long(0xffffffffUL, 0xffffffffUL));
}

TEST(test_add_overflow_long_long) {
  TEST_ASSERT(add_overflow_long_long(LLONG_MAX, 1));
  TEST_ASSERT(!add_overflow_long_long(LLONG_MAX, 0));

  TEST_ASSERT(add_overflow_long_long(1, LLONG_MAX));
  TEST_ASSERT(!add_overflow_long_long(0, LLONG_MAX));

  TEST_ASSERT(add_overflow_long_long(LLONG_MIN, -1));
  TEST_ASSERT(!add_overflow_long_long(LLONG_MIN, 0));

  TEST_ASSERT(add_overflow_long_long(-1, LLONG_MIN));
  TEST_ASSERT(!add_overflow_long_long(0, LLONG_MIN));

  TEST_ASSERT(add_overflow_long_long(LLONG_MAX, LLONG_MAX));
  TEST_ASSERT(add_overflow_long_long(LLONG_MIN, LLONG_MIN));
  TEST_ASSERT(!add_overflow_long_long(LLONG_MAX, LLONG_MIN));

  TEST_ASSERT(!add_overflow_long_long(1, 1));
  TEST_ASSERT(!add_overflow_long_long(1, -1));
  TEST_ASSERT(!add_overflow_long_long(-1, -1));
}
