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

#include "process.h"
#include "lib/string.h"
#include "lib/list.h"
#include "lib/errno.h"
#include "lib/termios.h"
#include "lib/bitset.h"
#include "lib/limits.h"
#include "lib/arithmetic.h"
#include "slab.h"
#include "tty.h"
#include "system.h"
#include "mmu.h"
#include "elf.h"
#include "buddy.h"
#include "logger.h"
#include "fs.h"
#include "dentry.h"
#include "inode.h"
#include "file.h"
#include "user.h"

#define MAX_PROCESS_SIZE 8
#define MAX_FD_SIZE      32

#define SEGMENT_TYPE_SIZE  4

#define SEGMENT_TYPE_TEXT  0
#define SEGMENT_TYPE_DATA  1
#define SEGMENT_TYPE_STACK 2
#define SEGMENT_TYPE_HEAP  3

#define SEGMENT_FLAGS_GROWSUP   (1 << 0)
#define SEGMENT_FLAGS_GROWSDOWN (1 << 1)

#define STACK_START ((uint8_t*)0x58000000)
#define STACK_END   USER_ADDRESS_END

#define ARG_MAX (4 * 1024)
#define INITIAL_STACK_SIZE ARG_MAX

#define ALIGN(p, n) (((p) + ((1 << (n)) - 1)) & ~((1 << (n)) - 1))
#define PAGE_ALIGN(addr) (((addr) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))
#define PAGE_MASK(addr) ((addr) & ~(PAGE_SIZE - 1))

#define FILE_STATUS_FLAGS (O_APPEND|O_ASYNC|O_DIRECT|O_DSYNC|O_NOATIME|O_NONBLOCK|O_SYNC)
#define PIPE2_FLAGS (O_CLOEXEC|O_DIRECT|O_NONBLOCK)

#define KERNEL_STACK_SIZE (PAGE_SIZE * 4)

struct segment {
  uint8_t *start;
  uint8_t *current;
  uint8_t *end;
  uint32_t flags;
};

enum process_state {
  STATE_READY = 0,
  STATE_BLOCKED,
  STATE_DEAD,
};

struct process_signal {
  sigset_t mask;
  sigset_t pending;
  struct k_sigaction actions[NSIG-1];
};

struct process {
  struct list next;
  struct list task;
  struct list children;
  struct list sibling;
  pid_t id;
  struct process *parent;
  enum process_state state;
  int exit_status;
  struct process_context context;
  struct process_context *suspend;
  struct segment segments[SEGMENT_TYPE_SIZE];
  uint8_t *brk;
  uint8_t *kernel_stack;
  struct file *files[MAX_FD_SIZE];
  bitset close_on_exec[bitset_nslots(MAX_FD_SIZE)];
  struct process_signal signal;
};

struct process_waitq_entry {
  struct list next;
  struct process *process;
};

struct argv_envp {
  int size;
  int nr;
  int argc;
  char **argv;
  char **envp;
  struct page *page;
};

struct process *current_process;

static struct list all_processes;
static struct list run_queue;

static struct slab_cache *process_cache;
static struct slab_cache *file_cache;
static struct slab_cache *waitq_cache;
static struct slab_cache *context_cache;

static struct process_waitq child_waitq;
static struct termios terminal_config;

static struct process *find_process(pid_t pid) {
  struct process *p;

  list_foreach(p, &all_processes, next) {
    if (p->id == pid) {
      return p;
    }
  }

  return NULL;
}

static struct process *find_toplevel_process() {
  struct process *p;

  list_foreach(p, &all_processes, next) {
    if (p->parent == p) {
      return p;
    }
  }

  return NULL;
}

static void create_segments(struct process *process, const struct elf_executable *executable) {
  struct segment *segment;

  uint8_t *text_start = (uint8_t*)executable->text.addr;
  uint8_t *text_end   = (uint8_t*)PAGE_ALIGN(executable->text.addr + executable->text.memory_size);

  uint8_t *data_start = (uint8_t*)PAGE_MASK(executable->data.addr);
  uint8_t *data_end   = (uint8_t*)PAGE_ALIGN(executable->data.addr + executable->data.memory_size);

  segment = &process->segments[SEGMENT_TYPE_TEXT];
  segment->start   = text_start;
  segment->current = text_end;
  segment->end     = text_end;
  segment->flags   = SEGMENT_FLAGS_GROWSUP;

  segment = &process->segments[SEGMENT_TYPE_DATA];
  segment->start   = data_start;
  segment->current = data_end;
  segment->end     = data_end;
  segment->flags   = SEGMENT_FLAGS_GROWSUP;

  segment = &process->segments[SEGMENT_TYPE_HEAP];
  segment->start   = data_end ? data_end : text_end;
  segment->current = segment->start;
  segment->end     = segment->start;
  segment->flags   = SEGMENT_FLAGS_GROWSUP;

  segment = &process->segments[SEGMENT_TYPE_STACK];
  segment->start   = STACK_START;
  segment->current = STACK_END - INITIAL_STACK_SIZE;
  segment->end     = STACK_END;
  segment->flags   = SEGMENT_FLAGS_GROWSDOWN;
}

static struct segment *find_segment(uint8_t *address) {
  int i;
  struct segment *segment;

  for (i = 0; i < SEGMENT_TYPE_SIZE; ++i) {
    segment = &current_process->segments[i];

    if (address >= segment->start && address < segment->end) {
      return segment;
    }
  }

  return NULL;
}

static void alloc_segments(const struct process *process) {
  int i;
  const struct segment *segment;
  uint8_t *start, *end;

  for (i = 0; i < SEGMENT_TYPE_SIZE; ++i) {
    segment = &process->segments[i];

    if (segment->flags & SEGMENT_FLAGS_GROWSUP) {
      start = segment->start;
      end   = segment->current;
    } else if (segment->flags & SEGMENT_FLAGS_GROWSDOWN) {
      start = segment->current;
      end   = segment->end;
    } else {
      logger_fatal("bad segment flags: 0x%x", segment->flags);
      system_halt();
    }

    mmu_alloc(process->id, (uint32_t)start, (uint32_t)(end - start));
  }
}

static void copy_segment(pid_t pid, const struct segment *seg) {
  struct page *page;
  uint8_t *start, *end, *cur, *buf;

  if (seg->flags & SEGMENT_FLAGS_GROWSUP) {
    start = seg->start;
    end = seg->current;
  } else if (seg->flags & SEGMENT_FLAGS_GROWSDOWN) {
    start = seg->current;
    end = seg->end;
  } else {
    logger_fatal("bad segment flags: 0x%x", seg->flags);
    system_halt();
  }

  page = buddy_alloc(PAGE_SIZE);
  buf = page_address(page);

  for (cur = start; cur < end; cur += PAGE_SIZE) {
    memcpy(buf, cur, PAGE_SIZE);

    mmu_alloc(pid, (uint32_t)cur, PAGE_SIZE);
    mmu_set_ttb(pid);

    memcpy(cur, buf, PAGE_SIZE);
    mmu_set_ttb(current_process->id);
  }

  buddy_free(page);
}

static void copy_segments(struct process *process, const struct process *parent) {
  int i;

  for (i = 0; i < SEGMENT_TYPE_SIZE; ++i) {
    copy_segment(process->id, &parent->segments[i]);
  }
}

static int alloc_file(struct process *p) {
  int i;
  struct file *file;
  for (i = 0; i < MAX_FD_SIZE; ++i) {
    if (!p->files[i]) {
      file = slab_cache_alloc(file_cache);

      memset(file, 0, sizeof(struct file));
      file->count = 1;

      p->files[i] = file;
      return i;
    }
  }
  return -EMFILE;
}

static void release_file(struct file *file) {
  SYSTEM_BUG_ON(file->count == 0);
  file->count--;

  switch(file->type) {
    case FF_PIPE:
      pipe_release(file->pipe, file->flags);
      break;
    default:
      break;
  }

  if (!file->count) {
    slab_cache_free(file_cache, file);
  }
}

static void countup_file(struct file *file) {
  file->count++;

  switch(file->type) {
    case FF_PIPE:
      pipe_countup(file->pipe, file->flags);
      break;
    default:
      break;
  }
}

static int create_tty(struct process *p) {
  struct file *file = NULL;
  int fd = alloc_file(p);

  if (fd < 0) {
    return fd;
  }
  file = p->files[fd];

  file->type = FF_TTY;
  file->termios = &terminal_config;
  file->flags = O_RDWR;

  return 0;
}

static struct file *get_file(int fd) {
  if (fd < 0 || fd >= MAX_FD_SIZE) {
    return NULL;
  }

  return current_process->files[fd];
}

static struct process *process_alloc(void) {
  static pid_t max_id = 1;
  struct process *p = slab_cache_alloc(process_cache);

  memset(p, 0, sizeof(struct process));
  list_init(&p->task);
  list_init(&p->children);
  list_init(&p->sibling);

  p->id = max_id++;
  list_add(&all_processes, &p->next);

  p->kernel_stack = page_address(buddy_alloc(KERNEL_STACK_SIZE));
  return p;
}

static void process_destroy(struct process *p) {
  struct process *child, *toplevel;

  SYSTEM_BUG_ON(current_process->id == p->id);

  list_remove(&p->task);
  list_remove(&p->next);
  list_remove(&p->sibling);

  if (!list_empty(&p->children)) {
    toplevel = find_toplevel_process();

    if (toplevel) {
      list_foreach(child, &p->children, sibling) {
        child->parent = toplevel;
      }

      list_concat(&toplevel->children, &p->children);
    } else {
      list_foreach(child, &p->children, sibling) {
        child->parent = child;
      }
    }
  }

  buddy_free(page_find_by_address(p->kernel_stack));

  mmu_destroy(p->id);
  mmu_set_ttb(current_process->id);

  slab_cache_free(process_cache, p);
}

static int prepare_argv_and_envp(struct argv_envp *avep, char *const argv[], char *const envp[]) {
  int i, size;
  int argc = 0, nr = 2, nc = 0;
  char **uarg, *uchar, *s;

  for (i = 0; argv && argv[i]; ++i) {
    argc++;
    nr++;
    nc += strlen(argv[i]) + 1;
  }

  for (i = 0; envp && envp[i]; ++i) {
    nr++;
    nc += strlen(envp[i]) + 1;
  }

  size = ALIGN(sizeof(long) + nr * sizeof(char*) + nc * sizeof(char), 3);
  size = size < 0x100 ? 0x100 : size;

  if (size > ARG_MAX) {
    return -E2BIG;
  }

  avep->argc = argc;
  avep->size = size;
  avep->nr   = nr;
  avep->page = buddy_alloc(size);

  uarg  = (char**)page_address(avep->page);
  uchar = (char*)(uarg + nr);

  avep->argv = (char**)uarg;
  for (i = 0; argv && argv[i]; ++i) {
    s = argv[i];
    *uarg++ = uchar;
    while((*uchar++ = *s++));
  }
  *uarg++ = NULL;

  avep->envp = (char**)uarg;
  for (i = 0; envp && envp[i]; ++i) {
    s = envp[i];
    *uarg++ = uchar;
    while((*uchar++ = *s++));
  }
  *uarg++ = NULL;

  return 0;
}

static void *copy_argv_and_envp(const struct argv_envp *avep, char *ustack) {
  int i;
  char **uarg, *uchar, *s;

  int size = avep->size, nr = avep->nr;
  char **argv = avep->argv, **envp = avep->envp;

  uarg  = (char**)(ustack - size);
  uchar = (char*)(uarg + nr) + sizeof(long);

  *((long*)uarg) = avep->argc;
  uarg = (void*)((char*)uarg + sizeof(long));

  for (i = 0; argv && argv[i]; ++i) {
    s = argv[i];
    *uarg++ = uchar;
    while((*uchar++ = *s++));
  }
  *uarg++ = NULL;

  for (i = 0; envp && envp[i]; ++i) {
    s = envp[i];
    *uarg++ = uchar;
    while((*uchar++ = *s++));
  }
  *uarg++ = NULL;

  return (ustack - size);
}

static void release_argv_and_envp(struct argv_envp *avep) {
  if (avep->page) {
    buddy_free(avep->page);
    avep->page = NULL;
  }
}

static ssize_t emulate_readv(int fd, const struct iovec *iov, int iovcnt) {
  int i;
  ssize_t nread = 0, n;

  for (i = 0; i < iovcnt; ++i) {
    if (iov[i].iov_len) {
      n = process_read(fd, iov[i].iov_base, iov[i].iov_len);
      if (n < 0) {
        nread = n;
        break;
      }
      nread += n;

      if (n == 0 || iov[i].iov_len != (uint32_t)n) {
        break;
      }
    }
  }

  return nread;
}

static ssize_t emulate_writev(int fd, const struct iovec *iov, int iovcnt) {
  int i;
  ssize_t nwritten = 0, n;

  for (i = 0; i < iovcnt; ++i) {
    if (iov[i].iov_len) {
      n = process_write(fd, iov[i].iov_base, iov[i].iov_len);
      if (n < 0) {
        nwritten = n;
        break;
      }
      nwritten += n;

      if (iov[i].iov_len != (uint32_t)n) {
        break;
      }
    }
  }

  return nwritten;
}

static int process_dequeue() {
  struct process *p;

  list_foreach(p, &run_queue, task) {
    list_remove(&p->task);
    list_init(&p->task);

    current_process = p;
    return 1;
  }

  current_process = NULL;
  return 0;
}

static uint32_t push_to_stack(uint32_t sp, void *data, size_t size) {
  uint32_t signal_sp = (sp - size) & ~((1 << 3) - 1);
  return (uint32_t)memcpy((void*)signal_sp, data, size);
}

static uint32_t pop_from_stack(uint32_t sp, void *data, size_t size) {
  memcpy(data, (void*)sp, size);
  return ALIGN(sp + size, 3);
}

void process_init(void) {
  current_process = NULL;

  process_cache = slab_cache_create("process", sizeof(struct process));
  file_cache    = slab_cache_create("file",    sizeof(struct file));
  waitq_cache   = slab_cache_create("waitq",   sizeof(struct process_waitq_entry));
  context_cache = slab_cache_create("context", sizeof(struct process_context));

  list_init(&all_processes);
  list_init(&run_queue);

  process_waitq_init(&child_waitq);
  memset(&terminal_config, 0, sizeof(struct termios));

  terminal_config.c_oflag = (OPOST|ONLCR);
  terminal_config.c_iflag = (ICRNL|IXON|IXANY|IMAXBEL);
  terminal_config.c_cflag = (CREAD|CS8|B9600);
  terminal_config.c_lflag = (ISIG|ICANON|IEXTEN|ECHO|ECHOE|ECHOCTL|ECHOKE);

  terminal_config.c_cc[VTIME]   = 0;
  terminal_config.c_cc[VMIN]    = 1;
  terminal_config.c_cc[VINTR]   = (0x1f & 'c');
  terminal_config.c_cc[VQUIT]   = 28;
  terminal_config.c_cc[VEOF]    = (0x1f & 'd');
  terminal_config.c_cc[VSUSP]   = (0x1f & 'z');
  terminal_config.c_cc[VKILL]   = (0x1f & 'u');
  terminal_config.c_cc[VERASE]  = 0177;
  terminal_config.c_cc[VWERASE] = (0x1f & 'w');
}

pid_t process_get_id(struct process* process) {
  return process->id;
}

struct process_context *process_get_context(struct process *process) {
  return &process->context;
}

void process_set_context(struct process *process, const struct process_context *context) {
  memcpy(&process->context, context, sizeof(struct process_context));
}

void *process_get_kernel_stack(struct process *process) {
  return process->kernel_stack + KERNEL_STACK_SIZE;
}

int process_create(const char *path) {
  pid_t old_pid;
  struct argv_envp avep;
  struct elf_executable executable;
  struct process *process;
  struct file *tty = NULL;
  void *stack;
  int r;

  char *const argv[] = {
    (void*)path,
    NULL,
  };

  if ((r = prepare_argv_and_envp(&avep, argv, NULL)) < 0) {
    return r;
  }

  if (elf_load(path, &executable) < 0) {
    release_argv_and_envp(&avep);
    return -EACCES;
  }

  process = process_alloc();
  process->parent = process;

  create_segments(process, &executable);
  process->brk = process->segments[SEGMENT_TYPE_HEAP].start;

  old_pid = mmu_set_ttb(process->id);
  alloc_segments(process);

  elf_copy(&executable);
  elf_release(&executable);

  stack = copy_argv_and_envp(&avep, (void*)STACK_END);
  release_argv_and_envp(&avep);

  process->context.cpsr = 0x00000010;
  process->context.sp   = (uint32_t)stack;
  process->context.pc   = executable.entry_point;

  if (create_tty(process) < 0 || (tty = process->files[0]) == NULL) {
    logger_fatal("broken file descriptors");
    system_halt();
  }

  countup_file(tty);
  process->files[1] = tty;

  countup_file(tty);
  process->files[2] = tty;

  mmu_set_ttb(old_pid);
  return process->id;
}

int process_exec(const char *path, char *const argv[], char *const envp[]) {
  int i, r;
  struct k_sigaction *ksa;
  struct elf_executable executable;
  struct argv_envp avep;
  struct process *process = current_process;
  void *stack;

  if ((r = prepare_argv_and_envp(&avep, argv, envp)) < 0) {
    return r;
  }

  if (elf_load(path, &executable) < 0) {
    release_argv_and_envp(&avep);
    return -EACCES;
  }

  create_segments(process, &executable);
  process->brk = process->segments[SEGMENT_TYPE_HEAP].start;

  for (i = 0; i < (NSIG-1); ++i) {
    ksa = &process->signal.actions[i];
    if (ksa->handler != SIG_DFL && ksa->handler != SIG_IGN) {
      ksa->handler = SIG_DFL;
    }
  }

  for (i = 0; i < MAX_FD_SIZE; ++i) {
    if (process->files[i] && bitset_test(process->close_on_exec, i)) {
      release_file(process->files[i]);
      process->files[i] = NULL;
      bitset_remove(process->close_on_exec, i);
    }
  }

  mmu_destroy(process->id);
  mmu_set_ttb(process->id);
  alloc_segments(process);

  elf_copy(&executable);
  elf_release(&executable);

  stack = copy_argv_and_envp(&avep, (void*)STACK_END);
  release_argv_and_envp(&avep);

  memset(&process->context, 0, sizeof(struct process_context));
  process->context.cpsr = 0x00000010;
  process->context.sp   = (uint32_t)stack;
  process->context.pc   = executable.entry_point;

  return 0;
}

pid_t process_fork(const struct process_context *context) {
  int i;
  struct process *process;

  process = process_alloc();

  list_add(&current_process->children, &process->sibling);
  process->parent = current_process;

  memcpy(&process->context, context, sizeof(struct process_context));
  process->context.r[0] = 0;

  process->brk = current_process->brk;
  memcpy(process->segments, current_process->segments, sizeof(struct segment) * SEGMENT_TYPE_SIZE);
  copy_segments(process, current_process);

  memcpy(process->files, current_process->files, sizeof(struct file*) * MAX_FD_SIZE);
  for (i = 0; i < MAX_FD_SIZE; ++i) {
    if (process->files[i]) {
      countup_file(process->files[i]);
    }
  }

  memcpy(&process->signal, &current_process->signal, sizeof(struct process_signal));
  return process->id;
}

void process_sleep(struct process_waitq *waitq) {
  struct process *process = current_process;
  struct process_waitq_entry *entry = slab_cache_alloc(waitq_cache);
  struct process_context *suspend = slab_cache_alloc(context_cache);

  entry->process = process;
  entry->process->state = STATE_BLOCKED;
  list_add(&waitq->next, &entry->next);

  memset(suspend, 0, sizeof(struct process_context));
  entry->process->suspend = suspend;

  system_suspend((uint32_t)suspend);
}

int process_wake(struct process_waitq *waitq) {
  struct process_waitq_entry *entry;

  list_foreach(entry, &waitq->next, next) {
    entry->process->state = STATE_READY;

    list_remove(&entry->next);
    slab_cache_free(waitq_cache, entry);

    return 1;
  }

  return 0;
}

void process_switch(void) {
  if (list_empty(&run_queue)) {
    process_schedule();
  }

  if (process_dequeue()) {
    process_dispatch();
  } else {
    system_idle();
  }
}

void process_schedule(void) {
  struct process *p;

  list_foreach(p, &all_processes, next) {
    if (p->state == STATE_READY) {
      list_add(&run_queue, &p->task);
    }
  }
}

pid_t process_wait(int *status) {
  int exit_status;
  struct process *p;
  pid_t child_pid;

  if (list_empty(&current_process->children)) {
    return -ECHILD;
  }

  while (1) {
    list_foreach(p, &current_process->children, sibling) {
      if (p->state == STATE_DEAD) {
        child_pid = p->id;
        exit_status = p->exit_status;

        process_destroy(p);

        *status = exit_status;
        return child_pid;
      }
    }

    process_sleep(&child_waitq);
  }
}

void process_exit(int status) {
  int i;
  struct file *file;
  struct process *p = current_process;

  p->state = STATE_DEAD;
  p->exit_status = status;

  for (i = 0; i < MAX_FD_SIZE; ++i) {
    file = p->files[i];

    if (file) {
      p->files[i] = NULL;
      release_file(file);
    }
  }

  if (list_length(&all_processes) == 1) {
    system_shutdown();
  }

  process_wake(&child_waitq);
}

void process_dispatch(void) {
  int sig;
  uint32_t signal_sp;
  sigset_t set;
  struct k_sigaction *ksa;
  struct process_context *context = &current_process->context;
  struct process_context suspend, *t_suspend;

  mmu_set_ttb(current_process->id);

  if (current_process->suspend) {
    t_suspend = current_process->suspend;
    current_process->suspend = NULL;

    memcpy(&suspend, t_suspend, sizeof(struct process_context));
    slab_cache_free(context_cache, t_suspend);

    system_resume((uint32_t)&suspend);
  }

  signotset(&set, &current_process->signal.mask);
  sigandset(&set, &current_process->signal.pending, &set);

  if ((sig = sigpeekset(&set))) {
    ksa = &current_process->signal.actions[sig];

    if (ksa->handler == SIG_DFL) {
      switch (sig) {
        case SIGCHLD:
        case SIGURG:
        case SIGWINCH:
          break;

        default:
          process_exit(WAIT_SIGNAL(sig));
          process_switch();
          break;
      }
    } else if (ksa->handler != SIG_IGN) {
      signal_sp = push_to_stack(context->sp, context, sizeof(struct process_context));
      signal_sp = push_to_stack(signal_sp, &current_process->signal.mask, sizeof(sigset_t));

      memset(context, 0, sizeof(struct process_context));
      context->r[0] = (uint32_t)sig;
      context->cpsr = 0x00000010;
      context->sp   = (uint32_t)signal_sp;
      context->lr   = (uint32_t)ksa->restorer;
      context->pc   = (uint32_t)ksa->handler;

      sigorset(&current_process->signal.mask, &current_process->signal.mask, &ksa->mask);
      sigaddset(&current_process->signal.mask, sig);
    }

    sigdelset(&current_process->signal.pending, sig);
  }

  system_dispatch((uint32_t)context);
}

pid_t process_getpid(void) {
  return current_process->id;
}

pid_t process_getppid(void) {
  return current_process->parent->id;
}

uint32_t process_brk(uint32_t address) {
  struct segment *segment = &current_process->segments[SEGMENT_TYPE_HEAP];
  uint32_t current_brk = (uint32_t)current_process->brk;
  pid_t pid = current_process->id;

  if ((address < current_brk) || (address >= (uint32_t)BRK_ADDRESS_END)) {
    return current_brk;
  }

  current_process->brk = (uint8_t*)address;

  if (segment->end < current_process->brk) {
    segment->end = (uint8_t*)PAGE_ALIGN((uint32_t)current_process->brk);

    while (segment->current < segment->end) {
      mmu_alloc(pid, (uint32_t)segment->current, PAGE_SIZE);
      segment->current += PAGE_SIZE;
    }
  }

  return address;
}

int process_open(const char *path, int flags, mode_t mode) {
  struct file *file = NULL;
  struct dentry *dentry = NULL;
  struct inode *inode = NULL;

  int r;
  int fd = alloc_file(current_process);

  if (fd < 0) {
    return fd;
  }
  file = current_process->files[fd];
  file->flags = (flags & (FILE_STATUS_FLAGS | O_ACCMODE));

  if (flags & O_CREAT) {
    r = fs_create(path, flags, mode);

    if (r < 0) {
      process_close(fd);
      return r;
    }
  }

  if (!(dentry = dentry_lookup(path))) {
    process_close(fd);
    return -ENOENT;
  }

  inode = dentry->inode;
  if (FILE_FOR_WRITE(file) && S_ISDIR(inode->mode)) {
    process_close(fd);
    return -EISDIR;
  }

  file->type   = FF_INODE;
  file->dentry = dentry;

  if (flags & O_TRUNC) {
    r = inode_truncate(dentry->inode, 0);
    SYSTEM_BUG_ON(r < 0);
  }

  if (flags & O_CLOEXEC) {
    bitset_add(current_process->close_on_exec, fd);
  }

  return fd;
}

int process_close(int fd) {
  struct file *file;

  file = get_file(fd);
  if (!file) {
    return -EBADF;
  }

  release_file(file);
  current_process->files[fd] = NULL;
  bitset_remove(current_process->close_on_exec, fd);

  return 0;
}

ssize_t process_writev(int fd, const struct iovec *iov, int iovcnt) {
  return emulate_writev(fd, iov, iovcnt);
}

ssize_t process_readv(int fd, const struct iovec *iov, int iovcnt) {
  return emulate_readv(fd, iov, iovcnt);
}

ssize_t process_write(int fd, const void *data, size_t size) {
  struct file *file;
  struct inode *inode;
  ssize_t r = 0;

  file = get_file(fd);
  if (!file) {
    return -EBADF;
  }

  if (!FILE_FOR_WRITE(file)) {
    return -EBADF;
  }

  switch(file->type) {
    case FF_TTY:
      return tty_write(data, size);

    case FF_INODE:
      inode = file->dentry->inode;

      if (file->flags & O_APPEND) {
        file->offset = inode->size;
      }

      if ((r = inode_write(inode, size, file->offset, data)) < 0) {
        return r;
      }

      file->offset += r;
      return r;

    case FF_PIPE:
      return pipe_write(file->pipe, data, size);

    default:
      logger_fatal("unknown file type: %d", file->type);
      system_halt();
  }
}

ssize_t process_read(int fd, void *data, size_t size) {
  struct file *file;
  struct inode *inode;
  ssize_t r = 0;

  file = get_file(fd);
  if (!file) {
    return -EBADF;
  }

  if (!FILE_FOR_READ(file)) {
    return -EBADF;
  }

  switch(file->type) {
    case FF_TTY:
      return tty_read(data, size);

    case FF_INODE:
      inode = file->dentry->inode;

      if (S_ISDIR(inode->mode)) {
        return -EISDIR;
      }

      if ((r = inode_read(file->dentry->inode, size, file->offset, data)) < 0) {
        return r;
      }
      file->offset += r;

      return r;

    case FF_PIPE:
      return pipe_read(file->pipe, data, size);

    default:
      logger_fatal("unknown file type: %d", file->type);
      system_halt();
  }
}

int process_getdents64(int fd, struct dirent64 *data, size_t size) {
  char *buf = (void*)data;
  int r, nread;
  size_t reclen, namelen;
  struct minix3_dirent minix3_dirent;
  struct file *file;
  struct inode *inode;
  struct dirent64 *dirent;

  file = get_file(fd);
  if (!file) {
    return -EBADF;
  }

  if (!FILE_FOR_READ(file)) {
    return -EBADF;
  }

  if (file->type != FF_INODE || !S_ISDIR(file->dentry->inode->mode)) {
    return -ENOTDIR;
  }
  inode = file->dentry->inode;

  while (1) {
    nread = inode_read(inode, sizeof(struct minix3_dirent), file->offset, &minix3_dirent);
    r = (buf - (char*)data);

    if (nread < 0) {
      return r ? r : nread;
    }

    if (nread == 0 || nread != sizeof(struct minix3_dirent)) {
      return r;
    }

    if (minix3_dirent.d_ino) {
      namelen = strnlen(minix3_dirent.d_name, NAME_MAX) + 1;
      reclen = (size_t)(&((struct dirent64*)NULL)->d_name);
      reclen = reclen + namelen;
      reclen = ALIGN(reclen, 3);
      if ((r + reclen) > size) {
        return r ? r : -EINVAL;
      }

      dirent = (void*)buf;
      memset(dirent, 0, sizeof(struct dirent64));

      dirent->d_ino = minix3_dirent.d_ino;
      dirent->d_off = file->offset + nread;
      dirent->d_reclen = reclen;
      dirent->d_type = 0;
      strncpy(dirent->d_name, minix3_dirent.d_name, namelen);

      if ((r + reclen) == size) {
        file->offset += nread;
        break;
      }
      buf += reclen;
    }
    file->offset += nread;
  }

  return size;
}

loff_t process_llseek(int fd, loff_t offset, int whence) {
  struct file *file;
  struct inode *inode;
  loff_t next_offset;

  file = get_file(fd);
  if (!file) {
    return -EBADF;
  }

  if (file->type != FF_INODE) {
    return -ESPIPE;
  }
  inode = file->dentry->inode;

  switch (whence) {
    case SEEK_SET:
      next_offset = offset;
      break;
    case SEEK_CUR:
      if (add_overflow_long_long(file->offset, offset)) {
        return -EOVERFLOW;
      }
      next_offset = file->offset + offset;
      break;
    case SEEK_END:
      if (add_overflow_long_long(inode->size, offset)) {
        return -EOVERFLOW;
      }
      next_offset = inode->size + offset;
      break;
    default:
      return -EINVAL;
  }

  if (next_offset < 0 || next_offset > ULONG_MAX) {
    return -EOVERFLOW;
  }

  file->offset = next_offset;
  return next_offset;
}

int process_fstat64(int fd, struct stat64 *buf) {
  struct file *file;

  file = get_file(fd);
  if (!file) {
    return -EBADF;
  }

  return fs_fstat64(file, buf);
}

int process_ioctl(int fd, unsigned long request, void *argp) {
  struct file *file;
  struct winsize *winsize;

  file = get_file(fd);
  if (!file) {
    return -EBADF;
  }

  if (file->type != FF_TTY) {
    return -ENOTTY;
  }

  switch (request) {
    case TCGETS:
      memcpy(argp, file->termios, sizeof(struct termios));
      return 0;
    case TCSETS:
    case TCSETSW:
    case TCSETSF:
      memcpy(file->termios, argp, sizeof(struct termios));
      return 0;
    case TIOCGWINSZ:
      winsize = argp;
      winsize->ws_row = 24;
      winsize->ws_col = 80;
      return 0;
  }

  logger_debug("unknown ioctl: fd=%d, request=0x%x", fd, request);
  return -EINVAL;
}

int process_fcntl64(int fd, int cmd, uint32_t *args) {
  struct file *file;
  struct process *process = current_process;

  file = get_file(fd);
  if (!file) {
    return -EBADF;
  }

  switch (cmd) {
    case F_DUPFD:
      return process_dupfd(fd, args[0], 0);

    case F_GETFD:
      return bitset_test(process->close_on_exec, fd) ? FD_CLOEXEC : 0;

    case F_SETFD:
      if (args[0] & FD_CLOEXEC) {
        bitset_add(process->close_on_exec, fd);
      } else {
        bitset_remove(process->close_on_exec, fd);
      }
      return 0;

    case F_GETFL:
      return (file->flags & (FILE_STATUS_FLAGS | O_ACCMODE));

    case F_SETFL:
      file->flags = ((args[0] & FILE_STATUS_FLAGS) | (file->flags & O_ACCMODE));
      return 0;
  }

  logger_debug("unknown fcntl: fd=%d, cmd=%d", fd, cmd);
  return -EINVAL;
}

int process_sigaction(int sig, struct k_sigaction *ksa, struct k_sigaction *ksa_old) {
  struct process_signal *signal = &current_process->signal;

  if (sig < 1 || sig >= NSIG) {
    return -EINVAL;
  }

  if (sig == SIGKILL || sig == SIGSTOP) {
    return -EINVAL;
  }

  if (ksa_old) {
    *ksa_old = signal->actions[sig];
  }

  if (ksa) {
    signal->actions[sig] = *ksa;
    sigdelset(&signal->actions[sig].mask, SIGKILL);
    sigdelset(&signal->actions[sig].mask, SIGSTOP);
  }

  return 0;
}

void process_sigreturn(struct process_context *context) {
  uint32_t signal_sp;

  signal_sp = pop_from_stack(context->sp, &current_process->signal.mask, sizeof(sigset_t));
  signal_sp = pop_from_stack(signal_sp, &current_process->context, sizeof(struct process_context));
}

int process_sigprocmask(int how, const sigset_t *set, sigset_t *oldset) {
  sigset_t *current = &current_process->signal.mask;
  sigset_t old = *current, tmp;

  if (set) {
    switch(how) {
      case SIG_BLOCK:
        sigorset(current, current, set);
        sigdelset(current, SIGKILL);
        sigdelset(current, SIGSTOP);
        break;

      case SIG_UNBLOCK:
        signotset(&tmp, set);
        sigandset(current, current, &tmp);
        break;

      case SIG_SETMASK:
        *current = *set;
        sigdelset(current, SIGKILL);
        sigdelset(current, SIGSTOP);
        break;

      default:
        return -EINVAL;
    }
  }

  if (oldset) {
    *oldset = old;
  }

  return 0;
}

int process_kill(pid_t pid, int sig) {
  struct process *process;

  if (sig < 1 || sig >= NSIG) {
    return -EINVAL;
  }

  if (pid < 1 || !(process = find_process(pid))) {
    return -ESRCH;
  }

  sigaddset(&process->signal.pending, sig);
  return 0;
}

int process_dupfd(int fd, int from, int flags) {
  int i;
  struct file *file;
  struct process *process = current_process;

  file = get_file(fd);
  if (!file) {
    return -EBADF;
  }

  if (from < 0 || from >= MAX_FD_SIZE) {
    return -EINVAL;
  }

  for (i = from; i < MAX_FD_SIZE; ++i) {
    if (process->files[i] == NULL) {
      countup_file(file);
      if (flags & O_CLOEXEC) {
        bitset_add(process->close_on_exec, i);
      }
      process->files[i] = file;
      return i;
    }
  }

  return -EMFILE;
}

int process_dup2(int oldfd, int newfd) {
  struct file *file;
  struct process *process = current_process;

  file = get_file(oldfd);
  if (!file) {
    return -EBADF;
  }

  if (newfd < 0 || newfd >= MAX_FD_SIZE) {
    return -EBADF;
  }

  if (oldfd == newfd) {
    return newfd;
  }

  if (process->files[newfd]) {
    process_close(newfd);
  }

  countup_file(file);
  process->files[newfd] = file;

  return newfd;
}

int process_dup3(int oldfd, int newfd, int flags) {
  struct file *file;
  struct process *process = current_process;

  file = get_file(oldfd);
  if (!file) {
    return -EBADF;
  }

  if (newfd < 0 || newfd >= MAX_FD_SIZE) {
    return -EBADF;
  }

  if (oldfd == newfd) {
    return -EINVAL;
  }

  if (process->files[newfd]) {
    process_close(newfd);
  }

  countup_file(file);
  if (flags & O_CLOEXEC) {
    bitset_add(process->close_on_exec, newfd);
  }
  process->files[newfd] = file;

  return newfd;
}

int process_pipe(int *pipefd) {
  return process_pipe2(pipefd, 0);
}

int process_pipe2(int *pipefd, int flags) {
  int rfd, wfd;
  struct file *rfile, *wfile;
  struct pipe *pipe;

  if (flags & ~PIPE2_FLAGS) {
    return -EINVAL;
  }

  rfd = alloc_file(current_process);
  if (rfd < 0) {
    return rfd;
  }

  wfd = alloc_file(current_process);
  if (wfd < 0) {
    process_close(rfd);
    return wfd;
  }

  pipe = pipe_create();
  rfile = current_process->files[rfd];
  wfile = current_process->files[wfd];

  rfile->type = wfile->type = FF_PIPE;
  rfile->pipe = wfile->pipe = pipe;
  rfile->flags = O_RDONLY | O_APPEND | (flags & PIPE2_FLAGS);
  wfile->flags = O_WRONLY | O_APPEND | (flags & PIPE2_FLAGS);

  if (flags & O_CLOEXEC) {
    bitset_add(current_process->close_on_exec, rfd);
    bitset_add(current_process->close_on_exec, wfd);
  }

  pipefd[0] = rfd;
  pipefd[1] = wfd;

  return 0;
}

bool process_demand_page(uint8_t *address) {
  uint8_t *base;

  pid_t pid = current_process->id;
  struct segment *segment = find_segment(address);

  if (!segment) {
    return false;
  }

  if (segment->flags & SEGMENT_FLAGS_GROWSDOWN) {
    base = (void*)PAGE_MASK((uint32_t)address);

    if (address < segment->start && address >= segment->current) {
      return false;
    }

    while (segment->current > base) {
      segment->current -= PAGE_SIZE;
      mmu_alloc(pid, (uint32_t)segment->current, PAGE_SIZE);
    }

    return true;
  }

  return false;
}

void process_waitq_init(struct process_waitq *waitq) {
  list_init(&waitq->next);
}
