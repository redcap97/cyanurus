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

#include "lib/type.h"
#include "list.h"

void list_init(struct list *list) {
  list->next = list;
  list->prev = list;
}

void list_add(struct list *list, struct list *item) {
  item->prev = list;
  item->next = list->next;
  list->next = item;
  item->next->prev = item;
}

void list_concat(struct list *list, struct list *other) {
  if (list_empty(other)) {
    return;
  }

  other->next->prev = list;
  other->prev->next = list->next;
  list->next->prev = other->prev;
  list->next = other->next;

  list_init(other);
}

void list_remove(struct list *item) {
  item->prev->next = item->next;
  item->next->prev = item->prev;
  item->next = NULL;
  item->prev = NULL;
}

int list_empty(const struct list *list) {
  return list->next == list;
}

int list_length(const struct list *list) {
  struct list *item;
  int count = 0;

  item = list->next;
  while (item != list) {
    item = item->next;
    count++;
  }
  return count;
}
