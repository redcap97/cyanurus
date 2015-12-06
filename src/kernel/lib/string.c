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

#include "lib/string.h"

size_t strlen(const char *str) {
  const char *s = str;
  while (*s++);
  return s - str - 1;
}

size_t strnlen(const char *str, size_t maxlen) {
  size_t i = 0;
  while (str[i] && i < maxlen) {
    i++;
  }
  return i;
}

char *strcpy(char *dest, const char *src) {
  char *s = dest;
  while ((*dest++ = *src++));
  return s;
}

char *strncpy(char *dest, const char *src, size_t size) {
  size_t i;

  for (i = 0; i < size && src[i] != '\0'; ++i) {
    dest[i] = src[i];
  }

  for (; i < size; ++i) {
    dest[i] = '\0';
  }

  return dest;
}

int strcmp(const char *s1, const char *s2) {
  while (*s1 || *s2) {
    if (*s1 != *s2) {
      return *s1 - *s2;
    }
    s1++;
    s2++;
  }
  return 0;
}

int strncmp(const char *s1, const char *s2, size_t size) {
  while (size-- && (*s1 || *s2)) {
    if (*s1 != *s2) {
      return *s1 - *s2;
    }
    s1++;
    s2++;
  }
  return 0;
}

void *memcpy(void *dest, const void *src, size_t size) {
  char *d = dest;
  const char *s = src;
  while(size--) {
    *d++ = *s++;
  }
  return dest;
}

void *memset(void *buf, int c, size_t size) {
  char *b = buf;
  while(size--) {
    *b++ = c;
  }
  return buf;
}

int memcmp(const void *buf1, const void *buf2, size_t n) {
  const char *s1 = buf1, *s2 = buf2;
  while (n--) {
    if (*s1 != *s2) {
      return *s1 - *s2;
    }
    s1++;
    s2++;
  }
  return 0;
}

char *strcat(char *dest, const char *src) {
  char *s = dest;

  while (*dest) {
    dest++;
  }
  while ((*dest++ = *src++));

  return s;
}

char *strncat(char *dest, const char *src, size_t size) {
  char *s = dest;

  while (*dest) {
    dest++;
  }

  while (size-- && *src != '\0') {
    *dest++ = *src++;
  }
  *dest = '\0';

  return s;
}

char *strchr(const char *s, int c) {
  char *r = strchrnul(s, c);
  unsigned char a = *(unsigned char*)r, b = (unsigned char)c;

  return a == b ? r : NULL;
}

char *strrchr(const char *s, int c) {
  size_t i = strlen(s);
  unsigned char *r = (unsigned char*)s, a = (unsigned char)c;

  do {
    if (r[i] == a) {
      return (char*)&r[i];
    }
  } while(i--);

  return NULL;
}

char *strchrnul(const char *s, int c) {
  unsigned char *r = (unsigned char*)s, a = (unsigned char)c;

  while (*r) {
    if (*r == a) {
      break;
    }
    r++;
  }
  return (char*)r;
}
