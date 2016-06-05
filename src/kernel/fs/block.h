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

#ifndef _CYANURUS_FS_BLOCK_H_
#define _CYANURUS_FS_BLOCK_H_

#include "lib/type.h"

#define BLOCK_SIZE (1024*4)

typedef uint16_t block_index;

void block_init(void);
void fs_block_read(block_index index, void *data);
void fs_block_write(block_index index, const void *data);

#endif
