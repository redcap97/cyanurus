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

#ifndef _CYANURUS_FS_H_
#define _CYANURUS_FS_H_

#include "lib/type.h"
#include "lib/unix.h"
#include "file.h"

void fs_init(void);
int fs_create(const char *path, int flags, mode_t mode);
int fs_unlink(const char *path);
int fs_mkdir(const char *path, mode_t mode);
int fs_rmdir(const char *path);
int fs_lstat64(const char *path, struct stat64 *buf);
int fs_fstat64(const struct file *file, struct stat64 *buf);

#endif
