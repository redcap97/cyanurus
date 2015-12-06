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

#ifndef _CYANURUS_USER_TEST_H_
#define _CYANURUS_USER_TEST_H_

#include <stdio.h>

#define TEST_MESSAGE_SIGNATURE "--a94e2gfwdd--"

#define TEST_START() \
  puts(":echo " TEST_MESSAGE_SIGNATURE);

#define TEST_FAIL()                     \
  do {                                  \
    char message[1024];                 \
    snprintf(message, sizeof(message),  \
      "$failure @%s() (%s:%u)",         \
      __func__, __FILE__, __LINE__      \
    );                                  \
                                        \
    puts("\n" TEST_MESSAGE_SIGNATURE);  \
    puts(message);                      \
    puts("$shudown");                   \
    while(1);                           \
  } while(0);

#define TEST_ASSERT(expr) \
  if (!(expr)) {          \
    TEST_FAIL();          \
  }

#define TEST_SUCCEED()                \
  puts("\n" TEST_MESSAGE_SIGNATURE);  \
  puts("$success");                   \
  puts("$shudown");                   \
  while(1);

#define TEST_CHECK()                  \
  puts("\n" TEST_MESSAGE_SIGNATURE);  \
  puts("$check");                     \
  puts("$shudown");                   \
  while(1);

#endif
