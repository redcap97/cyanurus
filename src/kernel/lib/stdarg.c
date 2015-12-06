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

#include "lib/stdarg.h"
#include "lib/string.h"
#include "lib/limits.h"

#define PLACEHOLDER_FLAG_ZERO (1 << 0)

struct placeholder {
  int width;
  int flags;
};

struct buffer {
  char *data;
  size_t size;
};

enum state {
  STATE_STRING,
  STATE_PLACEHOLDER_FLAG,
  STATE_PLACEHOLDER_WIDTH,
  STATE_PLACEHOLDER_TYPE,
};

static bool assign(struct buffer *buf, size_t index, char c) {
  if (index < buf->size) {
    buf->data[index] = c;
    return true;
  }
  return false;
}

static int compute_hex_width(unsigned int value) {
  int width = 0;

  do {
    width++;
    value >>= 4;
  } while (value);

  return width;
}

static int compute_signed_dec_width(int value) {
  int width = 0;

  if (value < 0) {
    width++;
  }

  do {
    width++;
    value /= 10;
  } while (value);

  return width;
}

static int compute_unsigned_dec_width(unsigned int value) {
  int width = 0;

  do {
    width++;
    value /= 10;
  } while (value);

  return width;
}

static size_t format_hex(size_t index, struct buffer *buf, const struct placeholder *ph, unsigned int value) {
  size_t rindex;
  int width = compute_hex_width(value), i;

  if (ph->width > width) {
    for (i = 0; i < (ph->width - width); ++i) {
      assign(buf, index++, (ph->flags & PLACEHOLDER_FLAG_ZERO) ? '0' :  ' ');
    }
  }

  rindex = index + (width - 1);
  do {
    assign(buf, rindex--, "0123456789abcdef"[value & 0xf]);
    value >>= 4;
  } while (value);

  return (index + width);
}

static size_t format_signed_dec(size_t index, struct buffer *buf, const struct placeholder *ph, int value) {
  size_t rindex;
  int width = compute_signed_dec_width(value), i;

  if (ph->flags & PLACEHOLDER_FLAG_ZERO) {
    if (value < 0) {
      assign(buf, index++, '-');
    }

    if (ph->width > width) {
      for (i = 0; i < (ph->width - width); ++i) {
        assign(buf, index++, '0');
      }
    }
  } else {
    if (ph->width > width) {
      for (i = 0; i < (ph->width - width); ++i) {
        assign(buf, index++, ' ');
      }
    }

    if (value < 0) {
      assign(buf, index++, '-');
    }
  }

  if (value < 0) {
    width--;
    value *= -1;
  }

  rindex = index + (width - 1);

  do {
    assign(buf, rindex--, "0123456789abcdef"[value % 10]);
    value /= 10;
  } while (value);

  return index + width;
}

static size_t format_unsigned_dec(size_t index, struct buffer *buf, const struct placeholder *ph, unsigned int value) {
  size_t rindex;
  int width = compute_unsigned_dec_width(value), i;

  if (ph->width > width) {
    for (i = 0; i < (ph->width - width); ++i) {
      assign(buf, index++, (ph->flags & PLACEHOLDER_FLAG_ZERO) ? '0' :  ' ');
    }
  }

  rindex = index + (width - 1);
  do {
    assign(buf, rindex--, "0123456789abcdef"[value % 10]);
    value /= 10;
  } while (value);

  return (index + width);
}

int vsnprintf(char *str, size_t size, const char *format, va_list ap) {
  enum state st = STATE_STRING;
  struct placeholder ph = { .width = 0, .flags = 0 };
  struct buffer buf = { .data = str, .size = size };

  char c;
  size_t index = 0;

  if (size > INT_MAX) {
    return EOF;
  }

  while ((c = *format++)) {
    switch (st) {
      case STATE_STRING:
        switch (c) {
          case '%':
            st = STATE_PLACEHOLDER_FLAG;
            break;

          default:
            assign(&buf, index++, c);
            break;
        }
        break;

      case STATE_PLACEHOLDER_FLAG:
        switch (c) {
          case '0':
            if (ph.flags & PLACEHOLDER_FLAG_ZERO) {
              return EOF;
            }

            ph.flags |= PLACEHOLDER_FLAG_ZERO;
            st = STATE_PLACEHOLDER_FLAG;
            continue;
        }

      case STATE_PLACEHOLDER_WIDTH:
        switch (c){
          case '0': case '1': case '2': case '3': case '4':
          case '5': case '6': case '7': case '8': case '9':
            ph.width = (ph.width * 10) + (c - '0');

            if (ph.width == 0) {
              return EOF;
            }

            st = STATE_PLACEHOLDER_WIDTH;
            continue;
        }

      case STATE_PLACEHOLDER_TYPE:
        switch (c) {
          case '%':
            if (ph.flags != 0 || ph.width != 0) {
              return EOF;
            }

            assign(&buf, index++, c);
            st = STATE_STRING;
            break;

          case 'c':
            if (ph.flags & PLACEHOLDER_FLAG_ZERO) {
              return EOF;
            }

            {
              int i;

              if (ph.width > 1) {
                for (i = 0; i < (ph.width - 1); ++i) {
                  assign(&buf, index++, ' ');
                }
              }

              assign(&buf, index++, (char)va_arg(ap, int));

              ph.flags = ph.width = 0;
              st = STATE_STRING;
            }
            break;

          case 's':
            if (ph.flags & PLACEHOLDER_FLAG_ZERO) {
              return EOF;
            }

            {
              char *s = va_arg(ap, char*);
              int i, len = strlen(s);

              if (ph.width > len) {
                for (i = 0; i < (ph.width - len); ++i) {
                  assign(&buf, index++, ' ');
                }
              }

              for (i = 0; i < len; ++i) {
                assign(&buf, index++, *s++);
              }

              ph.flags = ph.width = 0;
              st = STATE_STRING;
            }
            break;

          case 'x':
            if ((ph.flags & PLACEHOLDER_FLAG_ZERO) && ph.width == 0) {
              return EOF;
            }

            index = format_hex(index, &buf, &ph, va_arg(ap, unsigned int));

            ph.flags = ph.width = 0;
            st = STATE_STRING;

            break;

          case 'd':
            if ((ph.flags & PLACEHOLDER_FLAG_ZERO) && ph.width == 0) {
              return EOF;
            }

            index = format_signed_dec(index, &buf, &ph, va_arg(ap, int));

            ph.flags = ph.width = 0;
            st = STATE_STRING;

            break;

          case 'u':
            if ((ph.flags & PLACEHOLDER_FLAG_ZERO) && ph.width == 0) {
              return EOF;
            }

            index = format_unsigned_dec(index, &buf, &ph, va_arg(ap, unsigned int));

            ph.flags = ph.width = 0;
            st = STATE_STRING;

            break;

          default:
            return EOF;
        }
        break;
    }
  }

  if (st != STATE_STRING) {
    return EOF;
  }

  if (!assign(&buf, index++, '\0')) {
    assign(&buf, buf.size - 1, '\0');
  }

  return index - 1;
}

int snprintf(char *str, size_t size, const char *format, ...) {
  int ret;
  va_list ap;

  va_start(ap, format);
  ret = vsnprintf(str, size, format, ap);
  va_end(ap);

  return ret;
}

int vsprintf(char *str, const char *format, va_list ap) {
  return vsnprintf(str, INT_MAX, format, ap);
}

int sprintf(char *str, const char *format, ...) {
  int ret;
  va_list ap;

  va_start(ap, format);
  ret = vsprintf(str, format, ap);
  va_end(ap);

  return ret;
}
