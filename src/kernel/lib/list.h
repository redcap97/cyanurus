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

#ifndef _CYANURUS_LIB_LIST_H_
#define _CYANURUS_LIB_LIST_H_

#define container_of(ptr, type, member) \
  ((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))

#define list_foreach(item, head, member)                         \
  for (item = container_of((head)->next, typeof(*item), member); \
       &item->member != (head);                                  \
       item = container_of(item->member.next, typeof(*item), member))

#define list_foreach_safe(item, temp, head, member) \
  for (item = container_of((head)->next, typeof(*item), member),      \
       temp = container_of(item->member.next, typeof(*temp), member); \
       &item->member != (head);                                       \
       item = temp,                                                   \
       temp = container_of(item->member.next, typeof(*temp), member))

struct list {
  struct list *prev;
  struct list *next;
};

void list_init(struct list *list);
void list_add(struct list *list, struct list *item);
void list_concat(struct list *list, struct list *other);
void list_remove(struct list *item);
int list_empty(const struct list *list);
int list_length(const struct list *list);

#endif
