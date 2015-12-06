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

#include <lib/bitset.c>

#include "test.h"
#include "lib/bitset.t"

#include "lib/string.h"

#define BITSET_NBITS 32

TEST(test_bitset_sizeof) {
  TEST_ASSERT((sizeof(bitset) * 8) == BITSET_NBITS);
}

TEST(test_bitset_mask) {
  int i;

  for (i = 0; i < BITSET_NBITS; ++i) {
    TEST_ASSERT(bitset_mask(i) == (1U << i));
  }

  for (i = 0; i < BITSET_NBITS; ++i) {
    TEST_ASSERT(bitset_mask(BITSET_NBITS + i) == (1U << i));
  }
}

TEST(test_bitset_slot) {
  int i;

  for (i = 0; i < BITSET_NBITS; ++i) {
    TEST_ASSERT(bitset_slot(i) == 0);
  }

  for (i = 0; i < BITSET_NBITS; ++i) {
    TEST_ASSERT(bitset_slot(BITSET_NBITS + i) == 1);
  }
}

TEST(test_bitset_nslots) {
  int i;
  TEST_ASSERT(bitset_nslots(0) == 0);

  for (i = 0; i < 32; ++i) {
    TEST_ASSERT(bitset_nslots(i+1) == 1);
  }

  for (i = 0; i < 32; ++i) {
    TEST_ASSERT(bitset_nslots(i+BITSET_NBITS+1) == 2);
  }
}

TEST(test_bitset_add) {
  bitset s[2];
  memset(s, 0, sizeof(s));

  bitset_add(s, 0);
  bitset_add(s, 31);
  bitset_add(s, 32);
  bitset_add(s, 63);

  TEST_ASSERT(s[0] == 0x80000001);
  TEST_ASSERT(s[1] == 0x80000001);
}

TEST(test_bitset_remove) {
  bitset s[2];
  memset(s, 0xff, sizeof(s));

  bitset_remove(s, 0);
  bitset_remove(s, 31);
  bitset_remove(s, 32);
  bitset_remove(s, 63);

  TEST_ASSERT(s[0] == 0x7ffffffe);
  TEST_ASSERT(s[1] == 0x7ffffffe);
}

TEST(test_bitset_test) {
  bitset s[2] = {0x00000001, 0x00000001};

  TEST_ASSERT(bitset_test(s, 0));
  TEST_ASSERT(bitset_test(s, 32));

  TEST_ASSERT(!bitset_test(s, 1));
  TEST_ASSERT(!bitset_test(s, 33));
}
