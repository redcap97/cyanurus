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
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#define ALICE_TEXT \
  "Alice was beginning to get very tired of sitting by her sister on the bank, and of having nothing to do: once or twice she had peeped into the book her sister was reading, but it had no pictures or conversations in it, \"and what is the use of a book,\" thought Alice, \"without pictures or conversations?\"\n" \
  "So she was considering in her own mind, (as well as she could, for the hot day made her feel very sleepy and stupid,) whether the pleasure of making a daisy-chain would be worth the trouble of getting up and picking the daisies, when suddenly a white rabbit with pink eyes ran close by her.\n" \
  "There was nothing so very remarkable in that; nor did Alice think it so very much out of the way to hear the Rabbit say to itself, \"Oh dear! Oh dear! I shall be too late!\" (when she thought it over afterwards, it occurred to her that she ought to have wondered at this, but at the time it all seemed quite natural;) but when the Rabbit actually took a watch out of its waistcoat-pocket, and looked at it, and then hurried on, Alice started to her feet, for it flashed across her mind that she had never before seen a rabbit with either a waistcoat-pocket, or a watch to take out of it, and, burning with curiosity, she ran across the field after it, and was just in time to see it pop down a large rabbit-hole under the hedge.\n" \
  "In another moment down went Alice after it, never once considering how in the world she was to get out again.\n"

static void check_read_directory(void) {
  int fd;
  char buf[32];

  fd = open("/usr/share", O_RDONLY);
  TEST_ASSERT(fd >= 0);

  TEST_ASSERT(read(fd, buf, sizeof(buf)) == -1);
  TEST_ASSERT(errno == EISDIR);

  TEST_ASSERT(!close(fd));
}

static void check_read_file(void) {
  int fd;
  ssize_t size, total = 0;
  char buf[32], text[4096] = "";

  fd = open("/usr/share/alice.txt", O_RDONLY);
  TEST_ASSERT(fd >= 0);

  while (1) {
    size = read(fd, buf, sizeof(buf) - 1);
    TEST_ASSERT(size >= 0);

    if (!size) {
      break;
    }
    buf[size] = '\0';
    total += size;

    TEST_ASSERT((total + 1) <= (ssize_t)sizeof(text));
    strcat(text, buf);
  }

  TEST_ASSERT(!strcmp(ALICE_TEXT, text));
  TEST_ASSERT(!close(fd));
}

int main(void) {
  TEST_START();

  check_read_directory();
  check_read_file();

  TEST_SUCCEED();
  return 0;
}
