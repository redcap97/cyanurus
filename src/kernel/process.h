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

#ifndef _CYANURUS_PROCESS_H_
#define _CYANURUS_PROCESS_H_

#include "lib/list.h"
#include "lib/unix.h"

#define USER_ADDRESS_START ((uint8_t*)0x00010000)
#define USER_ADDRESS_END   ((uint8_t*)0x60000000)

#define WAIT_EXIT_CODE(status) (((status) & 0xff) << 8)
#define WAIT_SIGNAL(signal) ((signal) & 0x7f)
#define WAIT_CORE_DUMP(coredump) (((coredump) ? 1 : 0) << 7)

struct process_context {
  uint32_t cpsr;
  uint32_t r[13];
  uint32_t sp;
  uint32_t lr;
  uint32_t pc;
};

struct process_waitq {
  struct list next;
};

extern struct process *current_process;

void process_init(void);
pid_t process_get_id(struct process* process);

int process_create(const char *path);
int process_exec(const char *path, char *const argv[], char *const envp[]);
pid_t process_fork(const struct process_context *context);
void process_sleep(struct process_waitq *waitq);
int process_wake(struct process_waitq *waitq);
struct process_context *process_get_context(void);
void process_set_context(const struct process_context *context);
uint8_t *process_get_kernel_stack(void);
void process_switch(void);
void process_schedule(void);
pid_t process_wait(int *status);
void process_exit(int status);
void process_dispatch(void);
pid_t process_getpid(void);
pid_t process_getppid(void);
uint32_t process_brk(uint32_t address);

int process_open(const char *path, int flags, mode_t mode);
int process_close(int fd);
ssize_t process_write(int fd, const void *data, size_t size);
ssize_t process_read(int fd, void *data, size_t size);
ssize_t process_writev(int fd, const struct iovec *iov, int iovcnt);
ssize_t process_readv(int fd, const struct iovec *iov, int iovcnt);
int process_getdents64(int fd, struct dirent64 *data, size_t size);
int process_fstat64(int fd, struct stat64 *buf);
int process_ioctl(int fd, unsigned long request, void *argp);
int process_fcntl64(int fd, int cmd, uint32_t *args);

int process_sigaction(int signum, struct k_sigaction *ksa, struct k_sigaction *ksa_old);
int process_sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
void process_sigreturn(struct process_context *context);
int process_kill(pid_t pid, int sig);

int process_dupfd(int fd, int from, int flags);
int process_dup2(int oldfd, int newfd);
int process_dup3(int oldfd, int newfd, int flags);
int process_pipe(int *pipefd);
int process_pipe2(int *pipefd, int flags);

void process_waitq_init(struct process_waitq *waitq);
struct process *process_waitq_get_process(struct process_waitq *waitq);

bool process_validate_executable_address(void *address);

#endif
