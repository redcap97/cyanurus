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

#include "syscall.h"
#include "system.h"
#include "mmu.h"
#include "uart.h"
#include "lib/type.h"
#include "lib/unix.h"
#include "fs.h"
#include "fs/dentry.h"
#include "logger.h"
#include "lib/string.h"
#include "lib/errno.h"
#include "lib/termios.h"
#include "user.h"

static bool check_address_range(const void *p, size_t s) {
  const uint8_t *data = p;
  return IS_USER_ADDRESSS(data) && IS_USER_ADDRESSS(data + s);
}

static bool check_iovec(const struct iovec *iov, int iovcnt) {
  int i;

  for (i = 0; i < iovcnt; ++i) {
    if (!check_address_range(iov, sizeof(struct iovec)) || !check_address_range(iov->iov_base, iov->iov_len)) {
      return false;
    }
    iov++;
  }

  return true;
}

static bool check_string(const char *s) {
  do {
    if (!IS_USER_ADDRESSS(s)) {
      return false;
    }
  } while (*s++);

  return true;
}

static bool check_avep(char **argv) {
  if (!argv) {
    return true;
  }

  while (1) {
    if (!IS_USER_ADDRESSS(argv)) {
      return false;
    }

    if (!*argv) {
      return true;
    }

    if (!check_string(*argv)) {
      return false;
    }

    argv++;
  }
}

void syscall_exit(struct process_context *context) {
  uint32_t *args = &context->r[0];

  int status = args[0];
  process_exit(WAIT_EXIT_CODE(status));
}

void syscall_fork(struct process_context *context) {
  uint32_t *args = &context->r[0];
  args[0] = process_fork(context);
}

void syscall_read(struct process_context *context) {
  uint32_t *args = &context->r[0];

  int fd = args[0];
  void *data = (void*)args[1];
  size_t size = (size_t)args[2];

  if (!check_address_range(data, size)) {
    args[0] = -EFAULT;
    return;
  }

  args[0] = process_read(fd, data, size);
}

void syscall_write(struct process_context *context) {
  uint32_t *args = &context->r[0];

  int fd = args[0];
  const void *data = (const void*)args[1];
  size_t size = (size_t)args[2];

  if (!check_address_range(data, size)) {
    args[0] = -EFAULT;
    return;
  }

  args[0] = process_write(fd, data, size);
}

void syscall_open(struct process_context *context) {
  uint32_t *args = &context->r[0];

  const char *path = (const char*)args[0];
  int flags = args[1];
  mode_t mode = (mode_t)args[2];

  if (!check_string(path)) {
    args[0] = -EFAULT;
    return;
  }

  args[0] = process_open(path, flags, mode);
}

void syscall_close(struct process_context *context) {
  uint32_t *args = &context->r[0];

  int fd = args[0];
  args[0] = process_close(fd);
}

void syscall_unlink(struct process_context *context) {
  uint32_t *args = &context->r[0];
  const char *path = (const char*)context->r[0];

  if (!check_string(path)) {
    args[0] = -EFAULT;
    return;
  }

  args[0] = fs_unlink(path);
}

void syscall_execve(struct process_context *context) {
  int r;
  uint32_t *args = &context->r[0];

  const char *path = (const char*)args[0];
  char **argv = (char**)args[1];
  char **envp = (char**)args[2];

  if (!check_string(path) || !check_avep(argv) || !check_avep(envp)) {
    args[0] = -EFAULT;
    return;
  }

  if ((r = process_exec(path, argv, envp)) < 0) {
    context->r[0] = (uint32_t)r;
  }
}

void syscall_getpid(struct process_context *context) {
  uint32_t *args = &context->r[0];
  args[0] = process_getpid();
}

void syscall_getppid(struct process_context *context) {
  uint32_t *args = &context->r[0];
  args[0] = process_getppid();
}

void syscall_kill(struct process_context *context) {
  uint32_t *args = &context->r[0];

  pid_t pid = (pid_t)args[0];
  int sig = (int)args[1];

  args[0] = process_kill(pid, sig);
}

void syscall_mkdir(struct process_context *context) {
  uint32_t *args = &context->r[0];

  const char *path = (const char*)args[0];
  uint32_t mode = args[1];

  if (!check_string(path)) {
    args[0] = -EFAULT;
    return;
  }

  args[0] = fs_mkdir(path, mode);
}

void syscall_rmdir(struct process_context *context) {
  uint32_t *args = &context->r[0];
  const char *path = (const char*)args[0];

  if (!check_string(path)) {
    args[0] = -EFAULT;
    return;
  }

  args[0] = fs_rmdir(path);
}

void syscall_dup(struct process_context *context) {
  uint32_t *args = &context->r[0];
  int fd = args[0];
  args[0] = process_dupfd(fd, 0, 0);
}

void syscall_dup2(struct process_context *context) {
  uint32_t *args = &context->r[0];
  int oldfd = args[0];
  int newfd = args[1];
  args[0] = process_dup2(oldfd, newfd);
}

void syscall_dup3(struct process_context *context) {
  uint32_t *args = &context->r[0];
  int oldfd = args[0];
  int newfd = args[1];
  int flags = args[2];
  args[0] = process_dup3(oldfd, newfd, flags);
}

void syscall_pipe(struct process_context *context) {
  uint32_t *args = &context->r[0];
  int *pipefd = (int*)args[0];

  if (!check_address_range(pipefd, sizeof(int) * 2)) {
    args[0] = -EFAULT;
    return;
  }

  args[0] = process_pipe(pipefd);
}

void syscall_pipe2(struct process_context *context) {
  uint32_t *args = &context->r[0];
  int *pipefd = (int*)args[0];
  int flags = (int)args[1];

  if (!check_address_range(pipefd, sizeof(int) * 2)) {
    args[0] = -EFAULT;
    return;
  }

  args[0] = process_pipe2(pipefd, flags);
}

void syscall_brk(struct process_context *context) {
  uint32_t *args = &context->r[0];

  uint32_t address = (uint32_t)args[0];
  args[0] = process_brk(address);
}

void syscall_ioctl(struct process_context *context) {
  uint32_t *args = &context->r[0];
  int fd = (int)args[0];
  unsigned long request = (unsigned long)args[1];
  void *argp = (void*)args[2];

  switch (request) {
    case TCGETS:
    case TCSETS:
    case TCSETSW:
    case TCSETSF:
      if (!check_address_range(argp, sizeof(struct termios))) {
        goto fail;
      }
      break;
    case TIOCGWINSZ:
      if (!check_address_range(argp, sizeof(struct winsize))) {
        goto fail;
      }
      break;
  }

  args[0] = process_ioctl(fd, request, argp);
  return;

fail:
  args[0] = -EFAULT;
}

void syscall_rt_sigaction(struct process_context *context) {
  uint32_t *args = &context->r[0];

  int signum = (int)args[0];
  struct k_sigaction *ksa = (struct k_sigaction*)args[1];
  struct k_sigaction *ksa_old = (struct k_sigaction*)args[2];

  if (ksa && !check_address_range(ksa, sizeof(struct k_sigaction))) {
    goto fail;
  }

  if (ksa_old && !check_address_range(ksa_old, sizeof(struct k_sigaction))) {
    goto fail;
  }

  args[0] = process_sigaction(signum, ksa, ksa_old);
  return;

fail:
  args[0] = -EFAULT;
}

void syscall_rt_sigprocmask(struct process_context *context) {
  uint32_t *args = &context->r[0];

  int how = (int)args[0];
  const sigset_t *set = (const sigset_t *)args[1];
  sigset_t *oldset = (sigset_t *)args[2];

  if (set && !check_address_range(set, sizeof(sigset_t))) {
    goto fail;
  }

  if (oldset && !check_address_range(oldset, sizeof(sigset_t))) {
    goto fail;
  }

  args[0] = process_sigprocmask(how, set, oldset);
  return;

fail:
  args[0] = -EFAULT;
}

void syscall_wait4(struct process_context *context) {
  uint32_t *args = &context->r[0];
  int *status = (int*)args[1];

  if (!check_address_range(status, sizeof(int))) {
    args[0] = -EFAULT;
    return;
  }

  args[0] = process_wait(status);
}

void syscall_sigreturn(struct process_context *context) {
  process_sigreturn(context);
}

void syscall_uname(struct process_context *context) {
  uint32_t *args = &context->r[0];
  struct utsname *uts = (struct utsname *)args[0];

  if (!check_address_range(uts, sizeof(struct utsname))) {
    args[0] = -EFAULT;
    return;
  }

  args[0] = system_uname(uts);
}

void syscall_readv(struct process_context *context) {
  uint32_t *args = &context->r[0];

  int fd = args[0];
  const struct iovec *iov = (const struct iovec *)args[1];
  int iovcnt = args[2];

  if (!check_iovec(iov, iovcnt)) {
    args[0] = -EFAULT;
    return;
  }

  args[0] = process_readv(fd, iov, iovcnt);
}

void syscall_writev(struct process_context *context) {
  uint32_t *args = &context->r[0];

  int fd = args[0];
  const struct iovec *iov = (const struct iovec *)args[1];
  int iovcnt = args[2];

  if (!check_iovec(iov, iovcnt)) {
    args[0] = -EFAULT;
    return;
  }

  args[0] = process_writev(fd, iov, iovcnt);
}

void syscall_getcwd(struct process_context *context) {
  uint32_t *args = &context->r[0];

  char *buf = (char*)args[0];
  size_t size = (size_t)args[1];

  if (size < 2) {
    args[0] = -ERANGE;
    return;
  }

  if (!check_address_range(buf, size)) {
    args[0] = -EFAULT;
    return;
  }

  strcpy(buf, "/");
}

void syscall_stat64(struct process_context *context) {
  uint32_t *args = &context->r[0];

  char *path = (char*)args[0];
  struct stat64 *buf = (struct stat64*)args[1];

  if (!check_string(path) || !check_address_range(buf, sizeof(struct stat64))) {
    args[0] = -EFAULT;
    return;
  }

  args[0] = fs_lstat64(path, buf);
}

void syscall_fstat64(struct process_context *context) {
  uint32_t *args = &context->r[0];

  int fd = args[0];
  struct stat64 *buf = (struct stat64*)args[1];

  if (!check_address_range(buf, sizeof(struct stat64))) {
    args[0] = -EFAULT;
    return;
  }

  args[0] = process_fstat64(fd, buf);
}

void syscall_lstat64(struct process_context *context) {
  uint32_t *args = &context->r[0];

  char *path = (char*)args[0];
  struct stat64 *buf = (struct stat64*)args[1];

  if (!check_string(path) || !check_address_range(buf, sizeof(struct stat64))) {
    args[0] = -EFAULT;
    return;
  }

  args[0] = fs_lstat64(path, buf);
}

void syscall_getdents64(struct process_context *context) {
  uint32_t *args = &context->r[0];

  int fd = (int)args[0];
  struct dirent64 *data = (struct dirent64*)args[1];
  size_t size = (size_t)args[2];

  if (!check_address_range(data, size)) {
    args[0] = -EFAULT;
    return;
  }

  args[0] = process_getdents64(fd, data, size);
}

void syscall_fcntl64(struct process_context *context) {
  uint32_t *args = &context->r[0];

  int fd = (int)args[0];
  int cmd = (int)args[1];
  uint32_t *remaining = &args[2];

  args[0] = process_fcntl64(fd, cmd, remaining);
}

void syscall_pass(struct process_context *context, uint32_t ret) {
  uint32_t *args = &context->r[0];
  args[0] = ret;
}

void syscall_handler(void) {
  struct process_context *context = process_get_context();
  uint32_t number = context->r[7];

  switch(number) {
    case 1:   syscall_exit(context);           break;
    case 2:   syscall_fork(context);           break;
    case 3:   syscall_read(context);           break;
    case 4:   syscall_write(context);          break;
    case 5:   syscall_open(context);           break;
    case 6:   syscall_close(context);          break;
    case 10:  syscall_unlink(context);         break;
    case 11:  syscall_execve(context);         break;
    case 20:  syscall_getpid(context);         break;
    case 37:  syscall_kill(context);           break;
    case 39:  syscall_mkdir(context);          break;
    case 40:  syscall_rmdir(context);          break;
    case 41:  syscall_dup(context);            break;
    case 42:  syscall_pipe(context);           break;
    case 45:  syscall_brk(context);            break;
    case 54:  syscall_ioctl(context);          break;
    case 63:  syscall_dup2(context);           break;
    case 64:  syscall_getppid(context);        break;
    case 114: syscall_wait4(context);          break;
    case 119: syscall_sigreturn(context);      break;
    case 122: syscall_uname(context);          break;
    case 145: syscall_readv(context);          break;
    case 146: syscall_writev(context);         break;
    case 174: syscall_rt_sigaction(context);   break;
    case 175: syscall_rt_sigprocmask(context); break;
    case 183: syscall_getcwd(context);         break;
    case 195: syscall_stat64(context);         break;
    case 196: syscall_lstat64(context);        break;
    case 197: syscall_fstat64(context);        break;
    case 217: syscall_getdents64(context);     break;
    case 221: syscall_fcntl64(context);        break;
    case 358: syscall_dup3(context);           break;
    case 359: syscall_pipe2(context);          break;

    case 140: // _llseek
    case 248: // exit_group
    case 270: // fadvise64_64
      syscall_pass(context, 0);
      break;

    case 199: // getuid32
    case 200: // getgid32
    case 201: // geteuid32
    case 202: // getegid32
      syscall_pass(context, 1);
      break;

    default:
      logger_debug("unknown syscall: %d", number);
      syscall_pass(context, -EINVAL);
      break;
  }

  process_switch();
}
