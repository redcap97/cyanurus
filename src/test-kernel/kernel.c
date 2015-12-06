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

#include "test.h"
#include "uart.h"
#include "logger.h"
#include "lib/string.h"

#include "entries.h"

static void command_run(char *name) {
  struct test_entry *entry = test_entries;

  while (entry->name) {
    if (!strcmp(entry->name, name)) {
      test_run(entry->func);
      break;
    }
    entry++;
  }
}

static char *parse_command(char *cmd) {
  size_t len = strlen(cmd), i;
  char *arg = NULL;

  if (len && cmd[len-1] == '\n') {
    cmd[len-1] = '\0';
  }

  if (cmd[0] != '$' && cmd[0] != ':') {
    return NULL;
  }

  for (i = 0; i < len; ++i) {
    if (cmd[i] == ' ') {
      cmd[i] = '\0';
      arg = &cmd[i+1];
      break;
    }
  }

  return arg;
}

void kernel_main(void) {
  char cmd[64], *arg;

  while (1) {
    logger_info("$please");

    memset(cmd, 0, sizeof(cmd));
    uart_read(cmd, sizeof(cmd) - 1);

    arg = parse_command(cmd);

    if (!strcmp(cmd, "$run") && arg) {
      command_run(arg);
    }
  }
}
