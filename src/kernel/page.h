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

#ifndef _CYANURUS_PAGE_H_
#define _CYANURUS_PAGE_H_

#include "lib/list.h"

#define PAGE_START ((char*)(0x65000000))

#define PAGE_SIZE 0x1000
#define PAGE_NUM  0x4000

#define PAGE_MAX_ORDER 10
#define PAGE_MAX_DEPTH (PAGE_MAX_ORDER + 1)

#define PF_FREE_LIST  (1 << 0)
#define PF_FIRST_PAGE (1 << 1)

#define _page_cleanup_ _cleanup_(page_cleanup)

typedef unsigned long page_index;

struct page {
  page_index index;
  unsigned int flags;
  unsigned int order;
  struct page* next;
};

extern struct page *pages;

void page_init(void);
void *page_address(const struct page *page);
struct page *page_find_by_address(void *address);
struct page *page_find_head(const struct page *page);
void page_cleanup(struct page **page);

#endif
