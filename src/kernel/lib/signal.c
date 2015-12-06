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

#include "lib/signal.h"

int sigemptyset(sigset_t *set) {
  set->__fields[0] = 0;
  set->__fields[1] = 0;
  return 0;
}

int sigfillset(sigset_t *set) {
  set->__fields[0] = 0xffffffff;
  set->__fields[1] = 0xffffffff;
  return 0;
}

int sigaddset(sigset_t *set, int signum) {
  if (signum < 1 || signum >= NSIG) {
    return -1;
  }

  if (signum > 32) {
    set->__fields[1] |= (1 << (signum - 33));
  } else {
    set->__fields[0] |= (1 << (signum - 1));
  }

  return 0;
}

int sigdelset(sigset_t *set, int signum) {
  if (signum < 1 || signum >= NSIG) {
    return -1;
  }

  if (signum > 32) {
    set->__fields[1] &= ~(1 << (signum - 33));
  } else {
    set->__fields[0] &= ~(1 << (signum - 1));
  }

  return 0;
}

int sigismember(const sigset_t *set, int signum) {
  if (signum < 1 || signum >= NSIG) {
    return -1;
  }

  if (signum > 32) {
    return (set->__fields[1] & (1 << (signum - 33))) ? 1 : 0;
  } else {
    return (set->__fields[0] & (1 << (signum - 1))) ? 1 : 0;
  }
}

int sigisemptyset(const sigset_t *set) {
  return (!set->__fields[0] && !set->__fields[1]) ? 1 : 0;
}

int signotset(sigset_t *dest, const sigset_t *src) {
  dest->__fields[0] = ~src->__fields[0];
  dest->__fields[1] = ~src->__fields[1];
  return 0;
}

int sigorset(sigset_t *dest, const sigset_t *left, const sigset_t *right) {
  dest->__fields[0] = left->__fields[0] | right->__fields[0];
  dest->__fields[1] = left->__fields[1] | right->__fields[1];
  return 0;
}

int sigandset(sigset_t *dest, const sigset_t *left, const sigset_t *right) {
  dest->__fields[0] = left->__fields[0] & right->__fields[0];
  dest->__fields[1] = left->__fields[1] & right->__fields[1];
  return 0;
}

int sigpeekset(sigset_t *set) {
  int sig;

  if ((sig = __builtin_ffs(set->__fields[0])) > 0) {
    return sig;
  }

  if ((sig = __builtin_ffs(set->__fields[1])) > 0) {
    return sig + 32;
  }

  return 0;
}
