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

#ifndef _CYANURUS_IO_H_
#define _CYANURUS_IO_H_

#include "lib/list.h"
#include "lib/unix.h"

#define io_read32(a)     (*((volatile uint32_t *)(a)))
#define io_write32(a, v) (*((volatile uint32_t *)(a)) = (v))

#endif
