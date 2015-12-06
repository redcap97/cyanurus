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

#include <lib/signal.c>

#include "test.h"
#include "lib/signal.t"

TEST(test_sigemptyset) {
  sigset_t set;
  TEST_ASSERT(sigemptyset(&set) == 0);
  TEST_ASSERT(set.__fields[0] == 0);
  TEST_ASSERT(set.__fields[1] == 0);
}

TEST(test_sigfillset) {
  sigset_t set;
  TEST_ASSERT(sigfillset(&set) == 0);
  TEST_ASSERT(set.__fields[0] == 0xffffffff);
  TEST_ASSERT(set.__fields[1] == 0xffffffff);
}

TEST(test_sigaddset) {
  sigset_t set;
  sigemptyset(&set);

  TEST_ASSERT(sigaddset(&set, 0) == -1);
  TEST_ASSERT(sigaddset(&set, 8192) == -1);

  TEST_ASSERT(sigaddset(&set, 1) == 0);
  TEST_ASSERT(sigaddset(&set, 32) == 0);
  TEST_ASSERT(sigaddset(&set, 33) == 0);
  TEST_ASSERT(sigaddset(&set, 64) == 0);

  TEST_ASSERT(set.__fields[0] == ((1U << 0) | (1U << 31)));
  TEST_ASSERT(set.__fields[1] == ((1U << 0) | (1U << 31)));
}

TEST(test_sigdelset) {
  sigset_t set;
  sigfillset(&set);

  TEST_ASSERT(sigdelset(&set, 0) == -1);
  TEST_ASSERT(sigdelset(&set, 8192) == -1);

  TEST_ASSERT(sigdelset(&set, 1) == 0);
  TEST_ASSERT(sigdelset(&set, 32) == 0);
  TEST_ASSERT(sigdelset(&set, 33) == 0);
  TEST_ASSERT(sigdelset(&set, 64) == 0);

  TEST_ASSERT(set.__fields[0] == (0xffffffff & ~((1U << 0) | (1U << 31))));
  TEST_ASSERT(set.__fields[1] == (0xffffffff & ~((1U << 0) | (1U << 31))));
}

TEST(test_sigismember) {
  sigset_t set;
  sigemptyset(&set);

  TEST_ASSERT(sigdelset(&set, 0) == -1);
  TEST_ASSERT(sigdelset(&set, 8192) == -1);

  TEST_ASSERT(sigismember(&set, 1) == 0);
  sigaddset(&set, 1);
  TEST_ASSERT(sigismember(&set, 1) == 1);

  TEST_ASSERT(sigismember(&set, 33) == 0);
  sigaddset(&set, 33);
  TEST_ASSERT(sigismember(&set, 33) == 1);
}

TEST(test_sigisemptyset) {
  sigset_t set;

  sigemptyset(&set);
  TEST_ASSERT(sigisemptyset(&set) == 1);

  sigaddset(&set, 1);
  TEST_ASSERT(sigisemptyset(&set) == 0);

  TEST_ASSERT(sigdelset(&set, 1) == 0);
  TEST_ASSERT(sigdelset(&set, 32) == 0);
  TEST_ASSERT(sigdelset(&set, 33) == 0);
  TEST_ASSERT(sigdelset(&set, 64) == 0);
}

TEST(test_signotset) {
  sigset_t a, b;

  sigemptyset(&a);
  sigaddset(&a, 1);
  sigaddset(&a, 32);
  sigaddset(&a, 33);
  sigaddset(&a, 64);

  TEST_ASSERT(signotset(&b, &a) == 0);
  TEST_ASSERT(b.__fields[0] == 0x7ffffffe);
  TEST_ASSERT(b.__fields[1] == 0x7ffffffe);
}

TEST(test_sigorset) {
  sigset_t a, b, c;

  sigemptyset(&a);
  sigfillset(&b);
  sigorset(&c, &a, &b);

  TEST_ASSERT(c.__fields[0] == 0xffffffff);
  TEST_ASSERT(c.__fields[1] == 0xffffffff);
}

TEST(test_sigandset) {
  sigset_t a, b, c;

  sigemptyset(&a);
  sigfillset(&b);
  sigandset(&c, &a, &b);

  TEST_ASSERT(c.__fields[0] == 0);
  TEST_ASSERT(c.__fields[1] == 0);
}

TEST(test_sigpeekset) {
  sigset_t set;

  sigemptyset(&set);
  TEST_ASSERT(sigpeekset(&set) == 0);

  sigaddset(&set, 64);
  TEST_ASSERT(sigpeekset(&set) == 64);

  sigaddset(&set, 33);
  TEST_ASSERT(sigpeekset(&set) == 33);

  sigaddset(&set, 32);
  TEST_ASSERT(sigpeekset(&set) == 32);

  sigaddset(&set, 1);
  TEST_ASSERT(sigpeekset(&set) == 1);
}
