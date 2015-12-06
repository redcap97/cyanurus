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

#ifndef _CYANURUS_LIB_BITSET_H_
#define _CYANURUS_LIB_BITSET_H_

#include "lib/type.h"

typedef uint32_t bitset;

#define bitset_mask(n) (1U << ((n) % (sizeof(bitset) * 8)))
#define bitset_slot(n) ((n) / (sizeof(bitset) * 8))
#define bitset_nslots(n) (((n) + (sizeof(bitset) * 8) - 1) / (sizeof(bitset) * 8))
#define bitset_add(p, n) ((p)[bitset_slot(n)] |= bitset_mask(n))
#define bitset_remove(p, n) ((p)[bitset_slot(n)] &= ~bitset_mask(n))
#define bitset_test(p, n) ((p)[bitset_slot(n)] & bitset_mask(n))

#endif
