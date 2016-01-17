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

#include "lib/arithmetic.h"
#include "lib/limits.h"

bool add_overflow_unsigned_long(unsigned long a, unsigned long b) {
  return a > ULONG_MAX - b;
}

bool add_overflow_long_long(long long a, long long b) {
  return (b > 0 && a > LLONG_MAX - b) || (b < 0 && a < LLONG_MIN - b);
}
