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

#include "buddy.h"
#include "system.h"
#include "logger.h"
#include "lib/string.h"

struct free_list {
  struct page *head;
};

static struct free_list free_lists[PAGE_MAX_DEPTH];

static page_index buddy_find_buddy_index(struct page *page) {
  if ((page->index >> page->order) & 0x1) {
    return (page->index - (1 << page->order));
  } else {
    return (page->index + (1 << page->order));
  }
}

static struct page *buddy_list_pop(struct free_list *list) {
  struct page *page = list->head;

  if (page) {
    list->head = page->next;
    page->next = NULL;
    page->flags &= ~PF_FREE_LIST;
  }

  return page;
}

static void buddy_list_push(struct free_list *list, struct page *page) {
  if (list->head) {
    page->next = list->head;
  } else {
    page->next = NULL;
  }

  list->head = page;
  page->flags |= PF_FREE_LIST;
}

static struct page *buddy_list_remove(struct free_list *list, page_index index) {
  struct page **pp = &list->head, *page = list->head;

  while (page) {
    if (page->index == index) {
      *pp = page->next;

      page->next = NULL;
      page->flags &= ~PF_FREE_LIST;

      return page;
    }

    pp = &page->next;
    page = page->next;
  }

  return NULL;
}

static int buddy_is_free_buddy(const struct page *page, const struct page *buddy) {
  return ((buddy->flags & PF_FREE_LIST) && buddy->order == page->order);
}

void buddy_init(void) {
  struct page *page;
  int i, len, max_page_block = (1 << PAGE_MAX_ORDER);

  memset(free_lists, 0, sizeof(struct free_list) * PAGE_MAX_DEPTH);

  pages = (struct page*)(PAGE_START - (sizeof(struct page) * PAGE_NUM));
  memset(pages, 0, sizeof(struct page) * PAGE_NUM);

  for (i = 0; i < PAGE_NUM; ++i) {
    pages[i].index = i;
  }

  for (i = 0, len = PAGE_NUM / max_page_block; i < len; ++i) {
    page = &pages[i * max_page_block];

    if (i < (len - 1)) {
      page->next = &pages[(i + 1) * max_page_block];
    }

    page->order = PAGE_MAX_ORDER;
    page->flags |= PF_FREE_LIST;
  }

  free_lists[PAGE_MAX_ORDER].head = &pages[0];
}

static struct page* buddy_pull_block(int from) {
  int i, to = 0;
  struct free_list *list;
  struct page *page;

  for (i = from + 1; i < PAGE_MAX_DEPTH; ++i) {
    list = &free_lists[i];
    page = buddy_list_pop(list);

    if (page) {
      to = i - 1;
      break;
    }
  }

  if (i == PAGE_MAX_DEPTH) {
    logger_fatal("out of memory");
    system_halt();
  }

  for (i = to; i >= from; --i) {
    list = &free_lists[i];

    page->order = i;
    list->head = page;
    page->flags |= PF_FREE_LIST;

    page += (1 << i);
  }

  page->order = from;
  return page;
}

struct page *buddy_alloc(size_t size) {
  int i;
  struct free_list *list;
  struct page *page = NULL;

  for (i = 0; i < PAGE_MAX_DEPTH; ++i) {
    list = &free_lists[i];

    if (((1U << i) * PAGE_SIZE) >= size) {
      page = buddy_list_pop(list);

      if (!page) {
        page = buddy_pull_block(i);
      }

      break;
    }
  }

  if (i == PAGE_MAX_DEPTH) {
    logger_fatal("requested page is too large: size=0x%x", size);
    system_halt();
  }

  page->flags |= PF_FIRST_PAGE;

  return page;
}

void buddy_free(struct page *page) {
  struct free_list *list;
  struct page *p, *bp;
  page_index bi;

  list = &free_lists[page->order];

  bi = buddy_find_buddy_index(page);
  bp = &pages[bi];

  page->flags &= ~PF_FIRST_PAGE;

  if (page->order == PAGE_MAX_ORDER || !buddy_is_free_buddy(page, bp)) {
    buddy_list_push(list, page);
  } else {
    page->flags &= ~PF_FREE_LIST;
    page->next = NULL;

    p = buddy_list_remove(list, bi);
    if (!p) {
      logger_fatal("broken page");
      system_halt();
    }

    if ((bi >> page->order) & 0x1) {
      p->order = 0;
    } else {
      page->order = 0;
      page = p;
    }

    page->order++;
    return buddy_free(page);
  }
}
