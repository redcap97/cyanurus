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

#include <test.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char data[] = "??: data";

int main(void) {
  int st;
  pid_t pid;
  char stack[] = "??: stack";
  char *heap = NULL;

  TEST_START();

  heap = malloc(1024);
  TEST_ASSERT(heap != NULL);

  strcpy(heap, "??: heap");

  pid = fork();
  TEST_ASSERT(pid >= 0);

  if (!pid) {
    data[0]  = 'A';
    heap[0]  = 'A';
    stack[0] = 'A';

    puts(data);
    puts(heap);
    puts(stack);

    _exit(0);
  }
  wait(&st);

  data[1]  = 'B';
  heap[1]  = 'B';
  stack[1] = 'B';

  puts(data);
  puts(heap);
  puts(stack);

  TEST_CHECK();
  return 0;
}
