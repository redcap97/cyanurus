/*
Copyright 2015 Akira Midorikawa

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

#include "pipe.h"
#include "slab.h"
#include "page.h"
#include "file.h"
#include "buddy.h"
#include "logger.h"
#include "system.h"
#include "lib/string.h"
#include "lib/errno.h"

static struct slab_cache *pipe_cache;

void pipe_init(void) {
  pipe_cache = slab_cache_create("pipe", sizeof(struct pipe));
}

struct pipe *pipe_create(void) {
  struct pipe *pipe = memset(slab_cache_alloc(pipe_cache), 0, sizeof(struct pipe));

  pipe->page = buddy_alloc(PAGE_SIZE);

  process_waitq_init(&pipe->readers_waitq);
  process_waitq_init(&pipe->writers_waitq);

  pipe->readers = 1;
  pipe->writers = 1;

  return pipe;
}

ssize_t pipe_read(struct pipe *pipe, void *data, size_t size) {
  size_t remaining, start, length;
  ssize_t read = 0;
  char *data_buf = data;
  char *pipe_buf = page_address(pipe->page);

  while (true) {
    if (!pipe->length) {
      if (!pipe->writers) {
        return 0;
      }
      process_sleep(&pipe->readers_waitq);
      continue;
    }

    start = (pipe->offset + pipe->length) % PAGE_SIZE;
    remaining = start > pipe->offset ? (start - pipe->offset) : (PAGE_SIZE - pipe->offset);
    length = remaining > (size - read) ? (size - read) : remaining;
    memcpy(data_buf + read, pipe_buf + pipe->offset, length);

    read += length;
    pipe->offset = (pipe->offset + length) % PAGE_SIZE;
    pipe->length -= length;

    SYSTEM_BUG_ON(pipe->offset >= PAGE_SIZE);
    SYSTEM_BUG_ON(pipe->length >= PAGE_SIZE);

    process_wake(&pipe->writers_waitq);

    if (!pipe->length || size == (size_t)read) {
      return read;
    }
  }
}

ssize_t pipe_write(struct pipe *pipe, const void *data, size_t size) {
  size_t remaining, start, length;
  ssize_t written = 0;
  const char *data_buf = data;
  char *pipe_buf = page_address(pipe->page);

  while (true) {
    if (!pipe->readers) {
      process_kill(process_get_id(current_process), SIGPIPE);
      return -EPIPE;
    }

    if (pipe->length == PAGE_SIZE) {
      process_sleep(&pipe->writers_waitq);
      continue;
    }

    start = (pipe->offset + pipe->length) % PAGE_SIZE;
    remaining = start < pipe->offset ? (pipe->offset - start) : (PAGE_SIZE - start);
    length = remaining < (size - written) ? remaining : (size - written);
    memcpy(pipe_buf + start, data_buf + written, length);

    written += length;
    pipe->length += length;

    process_wake(&pipe->readers_waitq);
    SYSTEM_BUG_ON(pipe->length > PAGE_SIZE);

    if (size == (size_t)written) {
      return written;
    }
  }
}

void pipe_countup(struct pipe *pipe, int flags) {
  switch(flags & O_ACCMODE) {
    case O_RDONLY:
      pipe->readers++;
      break;
    case O_WRONLY:
      pipe->writers++;
      break;
    default:
      logger_fatal("unknown flags: 0x%08x", flags);
      system_halt();
  }
}

void pipe_release(struct pipe *pipe, int flags) {
  struct page *page;

  switch(flags & O_ACCMODE) {
    case O_RDONLY:
      SYSTEM_BUG_ON(pipe->readers == 0);
      pipe->readers -= 1;
      if (!pipe->readers) {
        while (process_wake(&pipe->writers_waitq));
      }
      break;
    case O_WRONLY:
      SYSTEM_BUG_ON(pipe->writers == 0);
      pipe->writers -= 1;
      if (!pipe->writers) {
        while (process_wake(&pipe->readers_waitq));
      }
      break;
    default:
      logger_fatal("unknown flags: 0x%08x", flags);
      system_halt();
  }

  if (!pipe->readers && !pipe->writers) {
    page = pipe->page;
    slab_cache_free(pipe_cache, pipe);
    buddy_free(page);
  }
}
