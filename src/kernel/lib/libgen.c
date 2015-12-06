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

#include "lib/libgen.h"
#include "lib/string.h"

char *dirname(char *path) {
  size_t i;

  if (!path || !(*path)) {
    return ".";
  }

  i = strlen(path) - 1;
  while (path[i] == '/') {
    if (!i--) {
      return "/";
    }
  }

  while (path[i] != '/') {
    if (!i--) {
      return ".";
    }
  }

  while (path[i] == '/') {
    if (!i--) {
      return "/";
    }
  }

  path[i+1] = '\0';
  return path;
}

char *basename(char *path) {
  size_t i;

  if (!path || !(*path)) {
    return ".";
  }

  i = strlen(path) - 1;
  while (i && path[i] == '/') {
    path[i--] = '\0';
  }

  while (i && path[i-1] != '/') {
    i--;
  }

  return path + i;
}
