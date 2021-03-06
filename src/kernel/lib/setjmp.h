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

#ifndef _CYANURUS_LIB_SETJMP_H_
#define _CYANURUS_LIB_SETJMP_H_

// r4 - lr
typedef unsigned long jmp_buf[11];

int setjmp(jmp_buf env);
void longjmp(jmp_buf env, int val);

#endif
