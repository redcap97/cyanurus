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

#ifndef _CYANURUS_LIB_UNIX_H_
#define _CYANURUS_LIB_UNIX_H_

#include "lib/type.h"
#include "lib/signal.h"

#define PIPE_BUF 4096

// for open
#define O_SEARCH  010000000
#define O_EXEC    010000000
#define O_PATH    010000000

#define O_ACCMODE (03|O_SEARCH)
#define O_RDONLY  00
#define O_WRONLY  01
#define O_RDWR    02

#define O_CREAT        0100
#define O_EXCL         0200
#define O_NOCTTY       0400
#define O_TRUNC       01000
#define O_APPEND      02000
#define O_NONBLOCK    04000
#define O_DSYNC      010000
#define O_SYNC     04010000
#define O_RSYNC    04010000
#define O_DIRECTORY  040000
#define O_NOFOLLOW  0100000
#define O_CLOEXEC  02000000

#define O_ASYNC      020000
#define O_DIRECT    0200000
#define O_LARGEFILE 0400000
#define O_NOATIME  01000000
#define O_TMPFILE 020040000
#define O_NDELAY O_NONBLOCK

// for fcntl
#define F_DUPFD  0
#define F_GETFD  1
#define F_SETFD  2
#define F_GETFL  3
#define F_SETFL  4

#define FD_CLOEXEC 1

// for ioctl
#define TCGETS     0x5401
#define TCSETS     0x5402
#define TCSETSW    0x5403
#define TCSETSF    0x5404
#define TIOCGWINSZ 0x5413
#define TIOCGPGRP  0x540f

typedef int32_t  pid_t;
typedef uint64_t dev_t;
typedef uint32_t mode_t;
typedef uint32_t nlink_t;
typedef uint32_t uid_t;
typedef uint32_t gid_t;
typedef int64_t  off_t;
typedef int64_t  off64_t;
typedef int64_t  loff_t;
typedef int32_t  blksize_t;
typedef int64_t  blkcnt_t;
typedef int64_t  blkcnt64_t;
typedef uint64_t ino_t;
typedef uint64_t ino64_t;
typedef int32_t  time_t;

struct timespec {
  time_t tv_sec;
  long tv_nsec;
};

struct stat64 {
  dev_t st_dev;
  int __st_dev_padding;
  long __st_ino_truncated;
  mode_t st_mode;
  nlink_t st_nlink;
  uid_t st_uid;
  gid_t st_gid;
  dev_t st_rdev;
  int __st_rdev_padding;
  off64_t st_size;
  blksize_t st_blksize;
  blkcnt64_t st_blocks;
  struct timespec st_atime;
  struct timespec st_mtime;
  struct timespec st_ctime;
  ino64_t st_ino;
};

struct dirent64 {
  ino64_t d_ino;
  off64_t d_off;
  unsigned short d_reclen;
  unsigned char d_type;
  char d_name[];
};

struct iovec {
  void *iov_base;
  size_t iov_len;
};

struct k_sigaction {
  void (*handler)(int);
  unsigned long flags;
  void (*restorer)(void);
  sigset_t mask;
};

struct utsname {
  char sysname[65];
  char nodename[65];
  char release[65];
  char version[65];
  char machine[65];
  char domainname[65];
};

#endif
