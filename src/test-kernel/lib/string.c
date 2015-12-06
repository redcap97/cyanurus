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

#include <lib/string.c>

#include "test.h"
#include "lib/string.t"

TEST(test_strlen) {
  TEST_ASSERT(strlen("") == 0);
  TEST_ASSERT(strlen("M1") == 2);
  TEST_ASSERT(strlen("M17") == 3);
}

TEST(test_strnlen) {
  TEST_ASSERT(strnlen("", 50) == 0);
  TEST_ASSERT(strnlen("M1", 50) == 2);
  TEST_ASSERT(strnlen("M17", 50) == 3);

  TEST_ASSERT(strnlen("M17", 0) == 0);
  TEST_ASSERT(strnlen("M17", 1) == 1);
  TEST_ASSERT(strnlen("M17", 2) == 2);
  TEST_ASSERT(strnlen("M17", 3) == 3);
  TEST_ASSERT(strnlen("M17", 4) == 3);
}

TEST(test_strcmp) {
  TEST_ASSERT(strcmp("M17", "M17") == 0);
  TEST_ASSERT(strcmp("M17", "M16") > 0);
  TEST_ASSERT(strcmp("M16", "M17") < 0);
}

TEST(test_strncmp) {
  TEST_ASSERT(strncmp("M17", "M17", 10) == 0);
  TEST_ASSERT(strncmp("M17", "M16", 10) > 0);
  TEST_ASSERT(strncmp("M16", "M17", 10) < 0);

  TEST_ASSERT(strncmp("M17", "M1", 2) == 0);
  TEST_ASSERT(strncmp("M17", "M1", 3) > 0);
  TEST_ASSERT(strncmp("M1", "M17", 3) < 0);
}

TEST(test_strcpy) {
  char buf[32];
  strcpy(buf, "M17");

  TEST_ASSERT(buf[0] == 'M');
  TEST_ASSERT(buf[1] == '1');
  TEST_ASSERT(buf[2] == '7');
  TEST_ASSERT(buf[3] == '\0');
}

TEST(test_strncpy) {
  char buf[32] = {0};

  strncpy(buf, "M17", 1);
  TEST_ASSERT(buf[0] == 'M');
  TEST_ASSERT(buf[1] == '\0');

  strncpy(buf, "M17", 10);
  TEST_ASSERT(buf[0] == 'M');
  TEST_ASSERT(buf[1] == '1');
  TEST_ASSERT(buf[2] == '7');
  TEST_ASSERT(buf[3] == '\0');
}

TEST(test_memcpy) {
  char b1[] = {0, 0, 0, 0};
  char b2[] = {1, 2, 3, 4};

  memcpy(b1, b2, 0);
  TEST_ASSERT(b1[0] == 0);

  memcpy(b1, b2, 3);
  TEST_ASSERT(b1[0] == 1);
  TEST_ASSERT(b1[1] == 2);
  TEST_ASSERT(b1[2] == 3);
  TEST_ASSERT(b1[3] == 0);
}

TEST(test_memset) {
  char buf[8] = {0};

  memset(buf, 10, 0);
  TEST_ASSERT(buf[0] == 0);

  memset(buf, 10, 1);
  TEST_ASSERT(buf[0] == 10);
  TEST_ASSERT(buf[1] == 0);
}

TEST(test_memcmp) {
  char b1[] = {10, 10};
  char b2[] = {10, 20};

  TEST_ASSERT(memcmp(b1, b2, 0) == 0);
  TEST_ASSERT(memcmp(b1, b2, 1) == 0);

  TEST_ASSERT(memcmp(b1, b2, 2) < 0);
  TEST_ASSERT(memcmp(b2, b1, 2) > 0);
}

TEST(test_strcat) {
  char buf[16] = "";

  strcat(buf, "");
  TEST_ASSERT(!strcmp(buf, ""));

  strcat(buf, "M1");
  TEST_ASSERT(!strcmp(buf, "M1"));

  strcat(buf, "7");
  TEST_ASSERT(!strcmp(buf, "M17"));
}

TEST(test_strncat) {
  char buf[16] = "";

  strncat(buf, "", 16);
  TEST_ASSERT(!strcmp(buf, ""));

  strncat(buf, "M17", 2);
  TEST_ASSERT(!strcmp(buf, "M1"));

  strncat(buf, "7", 16);
  TEST_ASSERT(!strcmp(buf, "M17"));
}

TEST(test_strchr) {
  const char *blank = "", *str = "M17M17";

  TEST_ASSERT(strchr(blank, 'A')  == NULL);
  TEST_ASSERT(strchr(blank, '\0') == &blank[0]);

  TEST_ASSERT(strchr(str, 'A')  == NULL);
  TEST_ASSERT(strchr(str, 'M')  == &str[0]);
  TEST_ASSERT(strchr(str, '7')  == &str[2]);
  TEST_ASSERT(strchr(str, '\0') == &str[6]);
}

TEST(test_strrchr) {
  const char *blank = "", *str = "M17M17";

  TEST_ASSERT(strrchr(blank, 'A')  == NULL);
  TEST_ASSERT(strrchr(blank, '\0') == &blank[0]);

  TEST_ASSERT(strrchr(str, 'A')  == NULL);
  TEST_ASSERT(strrchr(str, 'M')  == &str[3]);
  TEST_ASSERT(strrchr(str, '7')  == &str[5]);
  TEST_ASSERT(strrchr(str, '\0') == &str[6]);
}

TEST(test_strchrnul) {
  const char *blank = "", *str = "M17M17";

  TEST_ASSERT(strchrnul(blank, 'A')  == &blank[0]);
  TEST_ASSERT(strchrnul(blank, '\0') == &blank[0]);

  TEST_ASSERT(strchrnul(str, 'A')  == &str[6]);
  TEST_ASSERT(strchrnul(str, 'M')  == &str[0]);
  TEST_ASSERT(strchrnul(str, '7')  == &str[2]);
  TEST_ASSERT(strchrnul(str, '\0') == &str[6]);
}
