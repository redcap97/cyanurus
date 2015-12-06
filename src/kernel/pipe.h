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

#ifndef _CYANURUS_PIPE_H_
#define _CYANURUS_PIPE_H_

#include "lib/type.h"
#include "page.h"
#include "process.h"

struct pipe {
  struct page *page;
  size_t offset;
  size_t length;
  size_t readers;
  size_t writers;
  struct process_waitq readers_waitq;
  struct process_waitq writers_waitq;
};

void pipe_init(void);
struct pipe *pipe_create(void);
ssize_t pipe_read(struct pipe *pipe, void *data, size_t size);
ssize_t pipe_write(struct pipe *pipe, const void *data, size_t size);
void pipe_countup(struct pipe *pipe, int flags);
void pipe_release(struct pipe *pipe, int flags);

#endif
