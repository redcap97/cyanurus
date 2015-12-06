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

#include <lib/list.c>

#include "test.h"
#include "lib/string.t"

#include "lib/string.h"

struct entry {
  char *name;
  struct list next;
};

static struct list head;

static struct entry entries[] = {
  {"one",   { NULL, NULL }},
  {"two",   { NULL, NULL }},
  {"three", { NULL, NULL }},
};

static int count(const struct list *head) {
  int c = 0;
  struct list *item = head->next;

  while (item != head) {
    c++;
    item = item->next;
  }

  return c;
}

static int count_reverse(const struct list *head) {
  int c = 0;
  struct list *item = head->prev;

  while (item != head) {
    c++;
    item = item->prev;
  }

  return c;
}

TEST(test_list_init) {
  list_init(&head);

  TEST_ASSERT(head.next == &head);
  TEST_ASSERT(head.prev == &head);
}

TEST(test_list_add) {
  list_init(&head);
  TEST_ASSERT(count(&head)         == 0);
  TEST_ASSERT(count_reverse(&head) == 0);

  list_add(&head, &entries[0].next);
  TEST_ASSERT(count(&head)         == 1);
  TEST_ASSERT(count_reverse(&head) == 1);

  list_add(&head, &entries[1].next);
  TEST_ASSERT(count(&head)         == 2);
  TEST_ASSERT(count_reverse(&head) == 2);

  list_add(&head, &entries[2].next);
  TEST_ASSERT(count(&head)         == 3);
  TEST_ASSERT(count_reverse(&head) == 3);
}

TEST(test_list_concat_0) {
  struct list other_head;

  list_init(&head);
  list_init(&other_head);

  list_add(&head, &entries[0].next);
  list_concat(&head, &other_head);

  TEST_ASSERT(count(&head)         == 1);
  TEST_ASSERT(count_reverse(&head) == 1);

  TEST_ASSERT(list_empty(&other_head));
}

TEST(test_list_concat_1) {
  struct list other_head;

  list_init(&head);
  list_init(&other_head);

  list_add(&head, &entries[0].next);
  list_add(&other_head, &entries[1].next);
  list_add(&other_head, &entries[2].next);

  list_concat(&head, &other_head);

  TEST_ASSERT(count(&head)         == 3);
  TEST_ASSERT(count_reverse(&head) == 3);

  TEST_ASSERT(list_empty(&other_head));
}

TEST(test_list_remove) {
  struct list *list;

  list_init(&head);

  list_add(&head, &entries[0].next);
  list_add(&head, &entries[1].next);
  list_add(&head, &entries[2].next);

  TEST_ASSERT(count(&head)         == 3);
  TEST_ASSERT(count_reverse(&head) == 3);

  list = &entries[1].next;
  list_remove(list);

  TEST_ASSERT(count(&head)         == 2);
  TEST_ASSERT(count_reverse(&head) == 2);

  list = &entries[0].next;
  list_remove(list);

  TEST_ASSERT(count(&head)         == 1);
  TEST_ASSERT(count_reverse(&head) == 1);

  list = &entries[2].next;
  list_remove(list);

  TEST_ASSERT(count(&head)         == 0);
  TEST_ASSERT(count_reverse(&head) == 0);
}

TEST(test_list_empty) {
  list_init(&head);
  TEST_ASSERT(list_empty(&head));

  list_add(&head, &entries[0].next);
  TEST_ASSERT(!list_empty(&head));

  list_remove(&entries[0].next);
  TEST_ASSERT(list_empty(&head));
}

TEST(test_list_length) {
  list_init(&head);
  TEST_ASSERT(list_length(&head) == 0);

  list_add(&head, &entries[0].next);
  TEST_ASSERT(list_length(&head) == 1);

  list_add(&head, &entries[1].next);
  TEST_ASSERT(list_length(&head) == 2);

  list_add(&head, &entries[2].next);
  TEST_ASSERT(list_length(&head) == 3);
}

TEST(test_list_foreach) {
  struct entry *entry;
  int i = 0;

  list_init(&head);
  list_add(&head, &entries[2].next);
  list_add(&head, &entries[1].next);
  list_add(&head, &entries[0].next);

  list_foreach(entry, &head, next) {
    TEST_ASSERT(!strcmp(entry->name, entries[i].name));
    i++;
  }
}

TEST(test_list_foreach_safe) {
  struct entry *entry, *temp;
  int i = 0;

  list_init(&head);
  list_add(&head, &entries[2].next);
  list_add(&head, &entries[1].next);
  list_add(&head, &entries[0].next);

  list_foreach_safe(entry, temp, &head, next) {
    TEST_ASSERT(!strcmp(entry->name, entries[i].name));
    memset(entry, 0, sizeof(struct entry));

    i++;
  }
}
