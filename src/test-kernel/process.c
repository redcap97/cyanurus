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

#include <process.c>

#include "test.h"
#include "process.t"

#define INIT_PATH "/sbin/init"

static void setup(void) {
  page_init();
  fs_init();

  mmu_init();
  mmu_enable();

  process_init();
  pipe_init();
}

static struct process *get_process(pid_t pid) {
  struct process *p;

  list_foreach(p, &all_processes, next) {
    if (p->id == pid) {
      return p;
    }
  }

  TEST_FAIL();
  return NULL;
}

static void pseudo_switch_to(pid_t pid) {
  struct process *p = get_process(pid);

  current_process = p;
  mmu_set_ttb(p->id);
}

TEST(test_process_create_0) {
  int i, argc;
  pid_t pid;
  long *stack;
  char **avep;
  struct file *file;
  struct process *p;

  setup();
  pid = process_create(INIT_PATH);

  TEST_ASSERT(pid > 0);
  TEST_ASSERT(list_length(&all_processes) == 1);

  p = get_process(pid);

  TEST_ASSERT(p->state == STATE_READY);
  TEST_ASSERT(list_empty(&p->children));
  TEST_ASSERT(p->parent == p);

  for (i = 0; i < 3; ++i) {
    TEST_ASSERT((file = p->files[i]));

    TEST_ASSERT(file->type == FF_TTY);
    TEST_ASSERT(file->termios == &terminal_config);
    TEST_ASSERT(file->count == 3);
    TEST_ASSERT(file->flags == O_RDWR);
    TEST_ASSERT(file->offset == 0);
  }

  pseudo_switch_to(pid);
  stack = (void*)p->context.sp;
  argc = *stack;
  avep = (char**)(stack + 1);

  TEST_ASSERT(argc == 1);
  TEST_ASSERT(strcmp(avep[0], INIT_PATH) == 0);
  TEST_ASSERT(avep[1] == NULL);
}

TEST(test_process_create_1) {
  setup();
  TEST_ASSERT(process_create("/sbin/command_not_found") == -EACCES);
}

TEST(test_process_exec_0) {
  pid_t pid;
  int argc;
  long *stack;
  char s[0x100], **avep;

  char *argv[] = {
    INIT_PATH,
    s,
    NULL,
  };

  char *envp[] = {
    s,
    NULL,
  };

  memset(s, 'A', sizeof(s));
  s[sizeof(s) / 2] = '=';
  s[sizeof(s) - 1] = '\0';

  setup();

  pid = process_create(INIT_PATH);
  pseudo_switch_to(pid);

  TEST_ASSERT(process_exec(INIT_PATH, argv, envp) == 0);
  TEST_ASSERT(current_process->id == pid);

  stack = (void*)current_process->context.sp;
  argc = *stack;
  avep = (char**)(stack + 1);

  TEST_ASSERT(argc == 2);

  TEST_ASSERT(strcmp(argv[0], avep[0]) == 0);
  TEST_ASSERT(strcmp(argv[1], avep[1]) == 0);
  TEST_ASSERT(avep[2] == NULL);

  TEST_ASSERT(strcmp(envp[0], avep[3]) == 0);
  TEST_ASSERT(avep[4] == NULL);
}

TEST(test_process_exec_1) {
  pid_t pid;

  char *argv[] = {
    "/sbin/command_not_found",
    NULL,
  };

  char *envp[] = {
    "ABC=ABC",
    NULL,
  };

  setup();

  pid = process_create(INIT_PATH);
  pseudo_switch_to(pid);

  TEST_ASSERT(process_exec("/sbin/command_not_found", argv, envp) == -EACCES);
}

TEST(test_process_exec_2) {
  struct k_sigaction ksa;
  struct process *p;

  char *argv[] = {
    INIT_PATH,
    NULL,
  };

  char *envp[] = {
    NULL,
  };

  setup();

  p = get_process(process_create(INIT_PATH));
  pseudo_switch_to(p->id);

  TEST_ASSERT((unsigned long)p->segments[SEGMENT_TYPE_TEXT].start > 0);
  ksa.handler = (void (*)(int))p->segments[SEGMENT_TYPE_TEXT].start;
  TEST_ASSERT(process_sigaction(SIGINT, &ksa, NULL) == 0);

  TEST_ASSERT(p->signal.actions[SIGINT].handler != SIG_DFL);
  TEST_ASSERT(process_exec(INIT_PATH, argv, envp) == 0);
  TEST_ASSERT(p->signal.actions[SIGINT].handler == SIG_DFL);
}

TEST(test_process_exec_3) {
  char *argv[] = {
    NULL,
  };

  char *envp[] = {
    NULL,
  };

  struct process *p;

  setup();

  p = get_process(process_create(INIT_PATH));
  bitset_add(p->close_on_exec, 0);

  pseudo_switch_to(p->id);

  TEST_ASSERT(p->files[0] != NULL);
  TEST_ASSERT(bitset_test(p->close_on_exec, 0));

  TEST_ASSERT(process_exec(INIT_PATH, argv, envp) == 0);

  TEST_ASSERT(p->files[0] == NULL);
  TEST_ASSERT(!bitset_test(p->close_on_exec, 0));
}

TEST(test_process_fork) {
  int i;
  pid_t pid;
  struct file *file;
  struct process *p;

  setup();

  pid = process_create(INIT_PATH);
  pseudo_switch_to(pid);

  pid = process_fork(&current_process->context);
  TEST_ASSERT(pid > 0);
  TEST_ASSERT(list_length(&all_processes) == 2);

  p = get_process(pid);

  TEST_ASSERT(p->state == STATE_READY);
  TEST_ASSERT(list_empty(&p->children));

  TEST_ASSERT(list_length(&current_process->children) == 1);
  TEST_ASSERT(p->parent == current_process);

  for (i = 0; i < 3; ++i) {
    TEST_ASSERT((file = p->files[i]));

    TEST_ASSERT(file->type == FF_TTY);
    TEST_ASSERT(file->termios == &terminal_config);
    TEST_ASSERT(file->count == 6);
  }
}

TEST(test_process_destroy_0) {
  pid_t parent_pid, child_pid;
  struct process *parent;

  setup();

  parent_pid = process_create(INIT_PATH);
  pseudo_switch_to(parent_pid);
  parent = get_process(parent_pid);

  child_pid = process_fork(&parent->context);
  TEST_ASSERT(list_length(&parent->children) == 1);

  pseudo_switch_to(child_pid);
  process_destroy(get_process(child_pid));

  TEST_ASSERT(list_length(&parent->children) == 0);
}

TEST(test_process_destroy_1) {
  pid_t parent_pid, child_pid, grandchild_pid;
  struct process *parent, *child, *grandchild;

  setup();

  parent_pid = process_create(INIT_PATH);
  pseudo_switch_to(parent_pid);
  parent = get_process(parent_pid);

  child_pid = process_fork(&parent->context);
  pseudo_switch_to(child_pid);
  child = get_process(child_pid);

  grandchild_pid = process_fork(&child->context);
  pseudo_switch_to(grandchild_pid);
  grandchild = get_process(grandchild_pid);

  pseudo_switch_to(child_pid);
  process_destroy(child);

  TEST_ASSERT(grandchild->parent == parent);
  TEST_ASSERT(list_length(&parent->children) == 1);
}

TEST(test_process_destroy_2) {
  pid_t parent_pid, child_pid;
  struct process *parent, *child;

  setup();

  parent_pid = process_create(INIT_PATH);
  pseudo_switch_to(parent_pid);
  parent = get_process(parent_pid);

  child_pid = process_fork(&parent->context);
  pseudo_switch_to(child_pid);
  child = get_process(child_pid);

  pseudo_switch_to(parent_pid);
  process_destroy(parent);

  TEST_ASSERT(child->parent == child);
}

TEST(test_process_exit) {
  int i;
  pid_t child_pid;
  struct process *parent, *child;
  struct file *file;
  setup();

  parent = get_process(process_create(INIT_PATH));
  pseudo_switch_to(parent->id);

  child_pid = process_fork(&parent->context);

  for (i = 0; i < 3; ++i) {
    TEST_ASSERT((file = parent->files[i]));
    TEST_ASSERT(file->count == 6);
  }

  child = get_process(child_pid);
  pseudo_switch_to(child->id);

  process_exit(127);

  TEST_ASSERT(child->state == STATE_DEAD);
  TEST_ASSERT(child->exit_status == 127);

  for (i = 0; i < 3; ++i) {
    TEST_ASSERT((file = parent->files[i]));
    TEST_ASSERT(file->count == 3);
  }
}

TEST(test_process_schedule) {
  struct process *p;

  setup();

  p = get_process(process_create(INIT_PATH));
  p->state = STATE_READY;

  p = get_process(process_create(INIT_PATH));
  p->state = STATE_BLOCKED;

  TEST_ASSERT(list_length(&all_processes) == 2);
  TEST_ASSERT(list_length(&run_queue) == 0);

  process_schedule();

  TEST_ASSERT(list_length(&all_processes) == 2);
  TEST_ASSERT(list_length(&run_queue) == 1);
}

TEST(test_process_open_0) {
  int fd;
  struct process *p;
  struct file *file;

  setup();

  p = get_process(process_create(INIT_PATH));
  pseudo_switch_to(p->id);

  fd = process_open("/a.txt", O_CREAT | O_WRONLY | O_TRUNC, 0644);
  TEST_ASSERT(fd >= 0);

  TEST_ASSERT((file = p->files[fd]));

  TEST_ASSERT(file->type == FF_INODE);
  TEST_ASSERT(file->count == 1);
  TEST_ASSERT(file->flags == O_WRONLY);
  TEST_ASSERT(file->offset == 0);
}

TEST(test_process_open_1) {
  int fd;
  struct process *p;

  setup();

  p = get_process(process_create(INIT_PATH));
  pseudo_switch_to(p->id);

  fd = process_open("/a.txt", O_CREAT | O_WRONLY | O_TRUNC | O_CLOEXEC, 0644);
  TEST_ASSERT(fd >= 0);

  TEST_ASSERT(bitset_test(p->close_on_exec, fd));
}

TEST(test_process_close_0) {
  struct process *p;
  setup();

  p = get_process(process_create(INIT_PATH));
  bitset_add(p->close_on_exec, 0);

  pseudo_switch_to(p->id);

  TEST_ASSERT(p->files[0] != NULL);
  TEST_ASSERT(bitset_test(p->close_on_exec, 0));
  process_close(0);

  TEST_ASSERT(p->files[0] == NULL);
  TEST_ASSERT(!bitset_test(p->close_on_exec, 0));
}

TEST(test_process_close_1) {
  int fd;
  struct process *p;
  struct file *file;
  setup();

  p = get_process(process_create(INIT_PATH));
  pseudo_switch_to(p->id);

  fd = process_open("/a.txt", O_CREAT | O_WRONLY | O_TRUNC, 0644);
  TEST_ASSERT(fd >= 0);

  file = p->files[fd];
  TEST_ASSERT(file != NULL);

  TEST_ASSERT(process_dupfd(fd, 0, 0) >= 0);
  TEST_ASSERT(file->count == 2);

  process_close(fd);
  TEST_ASSERT(file->count == 1);
}

TEST(test_process_fstat64_0) {
  int fd;
  struct process *p;
  struct stat64 st;

  setup();

  p = get_process(process_create(INIT_PATH));
  pseudo_switch_to(p->id);

  fd = process_open("/", O_RDONLY|O_DIRECTORY, 0);
  TEST_ASSERT(fd >= 0);
  TEST_ASSERT(process_fstat64(fd, &st) == 0);
  TEST_ASSERT(S_ISDIR(st.st_mode));

  fd = process_open("/a.txt", O_CREAT | O_WRONLY | O_TRUNC, 0644);
  TEST_ASSERT(fd >= 0);
  TEST_ASSERT(process_fstat64(fd, &st) == 0);
  TEST_ASSERT(S_ISREG(st.st_mode));
}

TEST(test_process_fstat64_1) {
  struct process *p;
  struct stat64 st;

  setup();

  p = get_process(process_create(INIT_PATH));
  pseudo_switch_to(p->id);

  TEST_ASSERT(process_fstat64(100, &st) == -EBADF);

  TEST_ASSERT(process_fstat64(0, &st) == 0);
  TEST_ASSERT(S_ISCHR(st.st_mode));
}

TEST(test_process_sigaction) {
  struct process *p;
  struct k_sigaction ksa, ksa_old;

  setup();

  p = get_process(process_create(INIT_PATH));
  pseudo_switch_to(p->id);

  ksa.handler = SIG_IGN;

  TEST_ASSERT(process_sigaction(SIGKILL, &ksa, &ksa_old) == -EINVAL);
  TEST_ASSERT(process_sigaction(SIGSTOP, &ksa, &ksa_old) == -EINVAL);

  TEST_ASSERT(!process_sigaction(SIGINT, &ksa, &ksa_old));
  TEST_ASSERT(p->signal.actions[SIGINT].handler == SIG_IGN);
  TEST_ASSERT(ksa_old.handler == SIG_DFL);
}

TEST(test_process_sigprocmask_0) {
  struct process *p;
  sigset_t set, old;

  setup();

  p = get_process(process_create(INIT_PATH));
  pseudo_switch_to(p->id);
  TEST_ASSERT(sigisemptyset(&p->signal.mask));

  sigemptyset(&set);
  sigaddset(&set, SIGKILL);
  process_sigprocmask(SIG_BLOCK, &set, &old);
  TEST_ASSERT(sigisemptyset(&p->signal.mask));
  TEST_ASSERT(sigisemptyset(&old));

  sigemptyset(&set);
  sigaddset(&set, SIGINT);
  process_sigprocmask(SIG_BLOCK, &set, &old);
  TEST_ASSERT(sigismember(&p->signal.mask, SIGINT));
  TEST_ASSERT(sigisemptyset(&old));

  sigemptyset(&set);
  sigaddset(&set, SIGHUP);
  process_sigprocmask(SIG_BLOCK, &set, &old);
  TEST_ASSERT(sigismember(&p->signal.mask, SIGINT));
  TEST_ASSERT(sigismember(&p->signal.mask, SIGHUP));
  TEST_ASSERT(sigismember(&old, SIGINT));
  TEST_ASSERT(!sigismember(&old, SIGHUP));
}

TEST(test_process_sigprocmask_1) {
  struct process *p;
  sigset_t set, old;

  setup();

  p = get_process(process_create(INIT_PATH));
  pseudo_switch_to(p->id);
  TEST_ASSERT(sigisemptyset(&p->signal.mask));

  sigemptyset(&set);
  sigaddset(&set, SIGKILL);
  process_sigprocmask(SIG_SETMASK, &set, &old);
  TEST_ASSERT(sigisemptyset(&p->signal.mask));
  TEST_ASSERT(sigisemptyset(&old));

  sigemptyset(&set);
  sigaddset(&set, SIGINT);
  process_sigprocmask(SIG_SETMASK, &set, &old);
  TEST_ASSERT(sigismember(&p->signal.mask, SIGINT));
  TEST_ASSERT(sigisemptyset(&old));

  sigemptyset(&set);
  sigaddset(&set, SIGHUP);
  process_sigprocmask(SIG_SETMASK, &set, &old);
  TEST_ASSERT(!sigismember(&p->signal.mask, SIGINT));
  TEST_ASSERT(sigismember(&p->signal.mask, SIGHUP));
  TEST_ASSERT(sigismember(&old, SIGINT));
  TEST_ASSERT(!sigismember(&old, SIGHUP));
}

TEST(test_process_sigprocmask_2) {
  struct process *p;
  sigset_t set;

  setup();

  p = get_process(process_create(INIT_PATH));
  pseudo_switch_to(p->id);
  TEST_ASSERT(sigisemptyset(&p->signal.mask));

  sigfillset(&set);
  process_sigprocmask(SIG_SETMASK, &set, NULL);
  TEST_ASSERT(sigismember(&p->signal.mask, SIGINT));

  sigemptyset(&set);
  sigaddset(&set, SIGINT);
  process_sigprocmask(SIG_UNBLOCK, &set, NULL);
  TEST_ASSERT(!sigismember(&p->signal.mask, SIGINT));
}

TEST(test_process_sigprocmask_3) {
  struct process *p;
  sigset_t set, old;

  setup();

  p = get_process(process_create(INIT_PATH));
  pseudo_switch_to(p->id);
  TEST_ASSERT(sigisemptyset(&p->signal.mask));

  sigemptyset(&set);
  sigaddset(&set, SIGINT);
  process_sigprocmask(SIG_SETMASK, &set, NULL);

  process_sigprocmask(0, NULL, &old);
  TEST_ASSERT(!memcmp(&p->signal.mask, &old, sizeof(sigset_t)));
}

TEST(test_process_kill_0) {
  struct process *p;
  setup();

  p = get_process(process_create(INIT_PATH));
  pseudo_switch_to(p->id);
  TEST_ASSERT(sigisemptyset(&p->signal.pending));

  TEST_ASSERT(process_kill(p->id, SIGKILL) == 0);
  TEST_ASSERT(sigismember(&p->signal.pending, SIGKILL));
}

TEST(test_process_kill_1) {
  struct process *p;
  setup();

  p = get_process(process_create(INIT_PATH));
  pseudo_switch_to(p->id);

  TEST_ASSERT(process_kill(p->id, 0) == -EINVAL);
  TEST_ASSERT(process_kill(p->id, -1) == -EINVAL);
  TEST_ASSERT(process_kill(p->id, NSIG + 10) == -EINVAL);
  TEST_ASSERT(process_kill(p->id + 10, SIGINT) == -ESRCH);
}

TEST(test_process_dupfd_0) {
  int fd;
  struct process *p;
  struct file *file;
  setup();

  p = get_process(process_create(INIT_PATH));
  pseudo_switch_to(p->id);

  fd = process_open("/a.txt", O_CREAT | O_WRONLY | O_TRUNC, 0644);
  TEST_ASSERT(fd >= 0);

  file = p->files[fd];
  TEST_ASSERT(file != NULL);
  TEST_ASSERT(file->count == 1);

  TEST_ASSERT(process_dupfd(fd, 0, 0) == (fd+1));
  TEST_ASSERT(!bitset_test(p->close_on_exec, fd+1));
  TEST_ASSERT(file->count == 2);

  TEST_ASSERT(process_dupfd(fd, 0, O_CLOEXEC) == (fd+2));
  TEST_ASSERT(bitset_test(p->close_on_exec, fd+2));
  TEST_ASSERT(file->count == 3);

  TEST_ASSERT(process_dupfd(fd, 10, 0) == 10);
  TEST_ASSERT(!bitset_test(p->close_on_exec, 10));
  TEST_ASSERT(file->count == 4);
}

TEST(test_process_dupfd_1) {
  struct process *p;
  setup();

  p = get_process(process_create(INIT_PATH));
  pseudo_switch_to(p->id);

  TEST_ASSERT(process_dupfd(10, 0, 0) == -EBADF);

  TEST_ASSERT(process_dupfd(0, -1, 0) == -EINVAL);
  TEST_ASSERT(process_dupfd(0, MAX_FD_SIZE, 0) == -EINVAL);

  while(process_dupfd(0, 0, 0) != (MAX_FD_SIZE - 1));
  TEST_ASSERT(process_dupfd(0, 0, 0) == -EMFILE);
}

TEST(test_process_dup2_0) {
  int fd0, fd1;
  struct process *p;
  struct file *file0, *file1;
  setup();

  p = get_process(process_create(INIT_PATH));
  pseudo_switch_to(p->id);

  TEST_ASSERT(process_dup2(0, 0) == 0);

  fd0 = process_open("/a.txt", O_CREAT | O_WRONLY | O_TRUNC, 0644);
  TEST_ASSERT(fd0 >= 0);
  file0 = p->files[fd0];
  TEST_ASSERT(file0 != NULL);
  TEST_ASSERT(file0->count == 1);

  TEST_ASSERT(process_dup2(fd0, fd0+1) == (fd0+1));
  TEST_ASSERT(file0->count == 2);

  fd1 = process_open("/b.txt", O_CREAT | O_WRONLY | O_TRUNC, 0644);
  TEST_ASSERT(fd1 >= 0);
  file1 = p->files[fd1];
  TEST_ASSERT(file1 != NULL);
  TEST_ASSERT(file1->count == 1);

  TEST_ASSERT(process_dup2(fd1, fd0+1) == (fd0+1));
  TEST_ASSERT(file0->count == 1);
  TEST_ASSERT(file1->count == 2);
}

TEST(test_process_dup2_1) {
  struct process *p;
  setup();

  p = get_process(process_create(INIT_PATH));
  pseudo_switch_to(p->id);

  TEST_ASSERT(process_dup2(100, 0) == -EBADF);

  TEST_ASSERT(process_dup2(0, -1) == -EBADF);
  TEST_ASSERT(process_dup2(0, MAX_FD_SIZE) == -EBADF);
}

TEST(test_process_dup3_0) {
  int fd0, fd1;
  struct process *p;
  struct file *file0, *file1;
  setup();

  p = get_process(process_create(INIT_PATH));
  pseudo_switch_to(p->id);

  fd0 = process_open("/a.txt", O_CREAT | O_WRONLY | O_TRUNC, 0644);
  TEST_ASSERT(fd0 >= 0);
  file0 = p->files[fd0];
  TEST_ASSERT(file0 != NULL);
  TEST_ASSERT(file0->count == 1);

  TEST_ASSERT(process_dup3(fd0, fd0+1, O_CLOEXEC) == (fd0+1));
  TEST_ASSERT(file0->count == 2);
  TEST_ASSERT(bitset_test(p->close_on_exec, fd0+1));

  fd1 = process_open("/b.txt", O_CREAT | O_WRONLY | O_TRUNC, 0644);
  TEST_ASSERT(fd1 >= 0);
  file1 = p->files[fd1];
  TEST_ASSERT(file1 != NULL);
  TEST_ASSERT(file1->count == 1);

  TEST_ASSERT(process_dup3(fd1, fd0+1, 0) == (fd0+1));
  TEST_ASSERT(file0->count == 1);
  TEST_ASSERT(file1->count == 2);
  TEST_ASSERT(!bitset_test(p->close_on_exec, fd0+1));
}

TEST(test_process_dup3_1) {
  struct process *p;
  setup();

  p = get_process(process_create(INIT_PATH));
  pseudo_switch_to(p->id);

  TEST_ASSERT(process_dup3(100, 0, 0) == -EBADF);

  TEST_ASSERT(process_dup3(0, -1, 0) == -EBADF);
  TEST_ASSERT(process_dup3(0, MAX_FD_SIZE, 0) == -EBADF);

  TEST_ASSERT(process_dup3(0, 0, 0) == -EINVAL);
}

TEST(test_process_pipe2_0) {
  struct process *p;
  struct file *rfile, *wfile;
  int pipefd[2];

  setup();

  p = get_process(process_create(INIT_PATH));
  pseudo_switch_to(p->id);

  TEST_ASSERT(process_pipe2(pipefd, 0) == 0);

  rfile = current_process->files[pipefd[0]];
  wfile = current_process->files[pipefd[1]];

  TEST_ASSERT(rfile->type == FF_PIPE);
  TEST_ASSERT(rfile->count == 1);
  TEST_ASSERT(rfile->offset == 0);
  TEST_ASSERT(rfile->flags == (O_RDONLY | O_APPEND));

  TEST_ASSERT(wfile->type == FF_PIPE);
  TEST_ASSERT(wfile->count == 1);
  TEST_ASSERT(wfile->offset == 0);
  TEST_ASSERT(wfile->flags == (O_WRONLY | O_APPEND));

  TEST_ASSERT(rfile->pipe == wfile->pipe);
}

TEST(test_process_pipe2_1) {
  struct process *p;
  struct file *rfile, *wfile;
  int pipefd[2];

  setup();

  p = get_process(process_create(INIT_PATH));
  pseudo_switch_to(p->id);

  TEST_ASSERT(process_pipe2(pipefd, O_CLOEXEC|O_DIRECT|O_NONBLOCK) == 0);

  rfile = current_process->files[pipefd[0]];
  wfile = current_process->files[pipefd[1]];

  TEST_ASSERT(rfile->flags == (O_RDONLY|O_APPEND|O_CLOEXEC|O_DIRECT|O_NONBLOCK));
  TEST_ASSERT(wfile->flags == (O_WRONLY|O_APPEND|O_CLOEXEC|O_DIRECT|O_NONBLOCK));
}

TEST(test_process_pipe2_2) {
  struct process *p;
  int pipefd[2];

  setup();

  p = get_process(process_create(INIT_PATH));
  pseudo_switch_to(p->id);

  TEST_ASSERT(process_pipe2(pipefd, O_APPEND) == -EINVAL);
}

TEST(test_process_pipe2_3) {
  struct process *p;
  int pipefd[2];
  int fd = 0;

  setup();

  p = get_process(process_create(INIT_PATH));
  pseudo_switch_to(p->id);

  do {
    fd = process_open("/a.txt", O_CREAT | O_WRONLY | O_APPEND, 0644);
    TEST_ASSERT(fd >= 0);
  } while (fd < MAX_FD_SIZE - 2);

  TEST_ASSERT(process_pipe2(pipefd, 0) == -EMFILE);
}
