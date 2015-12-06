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

#include "test.h"
#include "logger.h"
#include "lib/setjmp.h"

#define MESSAGE_SIGNATURE "--a94e2gfwdd--"

enum test_flag {
  TEST_SUCCESS = 0,
  TEST_FAILURE
};

struct test_state {
  enum test_flag flag;

  struct {
    const char *file;
    unsigned int line;
    const char *func;
  } failure;
};

static struct test_state current_state;
static jmp_buf env;

static void report_each(void) {
  switch (current_state.flag) {
    case TEST_SUCCESS:
      logger_info("$success");
      break;

    case TEST_FAILURE:
      logger_info(
        "$failure @%s() (%s:%u)",
        current_state.failure.func,
        current_state.failure.file,
        current_state.failure.line
      );
      break;
  }
}

void test_run(void (*func)(void)) {
  current_state.flag = TEST_SUCCESS;

  test_message_start("echo");
  if (!setjmp(env)) {
    func();
  }
  test_message_stop();

  report_each();
}

void test_fail(const char *file, unsigned int line, const char *func) {
  current_state.flag = TEST_FAILURE;

  current_state.failure.file = file;
  current_state.failure.line = line;
  current_state.failure.func = func;

  longjmp(env, 1);
}

void test_message_start(const char *name) {
  logger_info(":%s %s", name, MESSAGE_SIGNATURE);
}

void test_message_stop(void) {
  logger_info("\n" MESSAGE_SIGNATURE);
}
