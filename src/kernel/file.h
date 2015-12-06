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

#ifndef _CYANURUS_FILE_H_
#define _CYANURUS_FILE_H_

#include <lib/unix.h>
#include <pipe.h>

#define FILE_FOR_READ(file)  (((file)->flags & O_ACCMODE) == O_RDWR || ((file)->flags & O_ACCMODE) == O_RDONLY)
#define FILE_FOR_WRITE(file) (((file)->flags & O_ACCMODE) == O_RDWR || ((file)->flags & O_ACCMODE) == O_WRONLY)

enum file_type {
  FF_UNUSED = 0,
  FF_TTY,
  FF_INODE,
  FF_PIPE,
};

struct file {
  enum file_type type;
  union {
    struct dentry *dentry;
    struct termios *termios;
    struct pipe *pipe;
  };
  mode_t flags;
  size_t offset;
  size_t count;
};

#endif
