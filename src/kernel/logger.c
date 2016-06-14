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

#include "logger.h"
#include "lib/string.h"
#include "lib/stdarg.h"
#include "uart.h"
#include "config.h"

/* In order to disable gcc warnings (-Wtype-limits) */
static const enum logger_level logger_level = CYANURUS_LOGGER_LEVEL;

static int logger_write(enum logger_level level, const char *format, va_list ap) {
  char str[128];

  if (level < logger_level) {
    return 0;
  }

  if (vsnprintf(str, sizeof(str) - 1, format, ap) == EOF) {
    return -1;
  }
  strcat(str, "\n");

  return uart_write(str, strlen(str));
}

int logger_debug(const char *format, ...) {
  int ret;
  va_list ap;

  va_start(ap, format);
  ret = logger_write(LOGGER_LEVEL_DEBUG, format, ap);
  va_end(ap);

  return ret;
}

int logger_info(const char *format, ...) {
  int ret;
  va_list ap;

  va_start(ap, format);
  ret = logger_write(LOGGER_LEVEL_INFO, format, ap);
  va_end(ap);

  return ret;
}

int logger_warn(const char *format, ...) {
  int ret;
  va_list ap;

  va_start(ap, format);
  ret = logger_write(LOGGER_LEVEL_WARN, format, ap);
  va_end(ap);

  return ret;
}

int logger_error(const char *format, ...) {
  int ret;
  va_list ap;

  va_start(ap, format);
  ret = logger_write(LOGGER_LEVEL_ERROR, format, ap);
  va_end(ap);

  return ret;
}

int logger_fatal(const char *format, ...) {
  int ret;
  va_list ap;

  va_start(ap, format);
  ret = logger_write(LOGGER_LEVEL_FATAL, format, ap);
  va_end(ap);

  return ret;
}
