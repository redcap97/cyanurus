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

#ifndef _CYANURUS_LIB_STRING_H_
#define _CYANURUS_LIB_STRING_H_

#include "lib/type.h"

size_t strlen(const char *str);
size_t strnlen(const char *str, size_t maxlen);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t size);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t size);
void *memcpy(void *dest, const void *src, size_t size);
void *memset(void *buf, int c, size_t size);
int memcmp(const void *buf1, const void *buf2, size_t n);
char *strcat(char *dest, const char *src);
char *strncat(char *dest, const char *src, size_t size);
char *strchr(const char *s, int c);
char *strrchr(const char *s, int c);
char *strchrnul(const char *s, int c);

#endif
