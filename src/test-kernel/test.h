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

#ifndef _CYANURUS_TEST_H_
#define _CYANURUS_TEST_H_

#define TEST(name) void name(void)

#define TEST_FAIL() \
  test_fail(__FILE__, __LINE__, __func__)

#define TEST_ASSERT(expr) \
  if (!(expr)) {          \
    TEST_FAIL();          \
  }

struct test_entry {
  const char *name;
  void (*func)(void);
};

#define TEST_ENTRY(entry) { #entry, entry }
#define TEST_ENTRY_NULL   { NULL,   NULL  }

void test_run(void (*func)(void));
void test_fail(const char *file, unsigned int line, const char *func);

void test_message_start(const char *name);
void test_message_stop(void);

#endif
