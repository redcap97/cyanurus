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

#include "elf.h"
#include "lib/string.h"
#include "lib/arithmetic.h"
#include "fs.h"
#include "fs/dentry.h"
#include "fs/inode.h"
#include "buddy.h"
#include "page.h"
#include "user.h"

#define ELF_MAGIC           "\177ELF"
#define ELF_CLASS_32BIT     1
#define ELF_DATA_2LE        1
#define ELF_VERSION_CURRENT 1
#define ELF_TYPE_EXEC       2
#define ELF_ARCH_ARM        40

#define ELF_FLAGS_EABI(flags) ((flags) & 0Xff000000)
#define ELF_FLAGS_EABI_V5     0x05000000

#define ELF_PH_TYPE_LOAD    1

#define ELF_PH_FLAGS_X      (1 << 0)
#define ELF_PH_FLAGS_W      (1 << 1)
#define ELF_PH_FLAGS_R      (1 << 2)

struct elf_header {
  struct {
    uint8_t magic[4];
    uint8_t class;
    uint8_t data;
    uint8_t version;
    uint8_t abi;
    uint8_t abi_version;
    uint8_t reserve[7];
  } id;
  uint16_t type;
  uint16_t arch;
  uint32_t version;
  uint32_t entry_point;
  uint32_t program_header_offset;
  uint32_t section_header_offset;
  uint32_t flags;
  uint16_t header_size;
  uint16_t program_header_size;
  uint16_t program_header_num;
  uint16_t section_header_size;
  uint16_t section_header_num;
  uint16_t section_name_index;
};

struct elf_program_header {
  uint32_t type;
  uint32_t offset;
  uint32_t virtual_addr;
  uint32_t physical_addr;
  uint32_t file_size;
  uint32_t memory_size;
  uint32_t flags;
  uint32_t align;
};

static bool validate_segment(const struct elf_segment *segment) {
  uint8_t *addr = (void*)segment->addr;
  uint32_t size = segment->memory_size;

  if (segment->file_size > segment->memory_size) {
    return false;
  }

  if (add_overflow_unsigned_long((unsigned long)addr, size)) {
    return false;
  }

  if (!IS_EXECUTABLE_ADDRESS(addr) || !IS_EXECUTABLE_ADDRESS(addr + size)) {
    return false;
  }

  return true;
}

static void copy_segment(struct elf_segment *segment) {
  void *data = page_address(segment->page);

  memcpy((char*)segment->addr, data, segment->file_size);
  memset((char*)segment->addr + segment->file_size, 0, segment->memory_size - segment->file_size);
}

static void release_segment(struct elf_segment *segment) {
  if (segment->page) {
    buddy_free(segment->page);
    segment->page = NULL;
  }
}

static int load_segment(struct elf_segment *segment, const struct elf_program_header *header, struct dentry *dentry) {
  void *data;
  ssize_t size;

  if (segment->page) {
    return -1;
  }

  segment->addr        = header->virtual_addr;
  segment->file_size   = header->file_size;
  segment->memory_size = header->memory_size;
  segment->page        = buddy_alloc(segment->file_size);

  if (!validate_segment(segment)) {
    goto fail;
  }

  data = page_address(segment->page);
  size = inode_read(dentry->inode, segment->file_size, header->offset, data);

  if (size < 0 || (uint32_t)size != segment->file_size) {
    goto fail;
  }

  return 0;
fail:
  release_segment(segment);
  return -1;
}

int elf_load(const char *path, struct elf_executable *executable) {
  int i, offset;
  struct dentry *dentry;
  ssize_t rs;
  struct elf_header header;
  struct elf_program_header program_header;

  if (!(dentry = dentry_lookup(path))) {
    return -1;
  }

  rs = inode_read(dentry->inode, sizeof(struct elf_header), 0, &header);
  if (rs != sizeof(struct elf_header)) {
    return -1;
  }

  if ((memcmp(header.id.magic, ELF_MAGIC, 4))      ||
        (header.id.class   != ELF_CLASS_32BIT)     ||
        (header.id.data    != ELF_DATA_2LE)        ||
        (header.id.version != ELF_VERSION_CURRENT) ||
        (header.type       != ELF_TYPE_EXEC)       ||
        (header.arch       != ELF_ARCH_ARM)        ||
        (header.version    != ELF_VERSION_CURRENT) ||
        (ELF_FLAGS_EABI(header.flags) != ELF_FLAGS_EABI_V5))
  {
    return -1;
  }

  memset(executable, 0, sizeof(struct elf_executable));

  for (i = 0; i < header.program_header_num; ++i) {
    offset = header.program_header_offset + (header.program_header_size * i);
    rs = inode_read(dentry->inode, sizeof(struct elf_program_header), offset, &program_header);

    if (rs != sizeof(struct elf_program_header)) {
      return -1;
    }

    if (program_header.type != ELF_PH_TYPE_LOAD) {
      continue;
    }

    switch(program_header.flags) {
      case (ELF_PH_FLAGS_R | ELF_PH_FLAGS_X):
        if (load_segment(&executable->text, &program_header, dentry) < 0) {
          goto fail;
        }
        break;

      case (ELF_PH_FLAGS_R | ELF_PH_FLAGS_W):
        if (load_segment(&executable->data, &program_header, dentry) < 0) {
          goto fail;
        }
        break;
    }
  }

  executable->entry_point = header.entry_point;
  return 0;
fail:
  elf_release(executable);
  return -1;
}

void elf_copy(struct elf_executable *executable) {
  copy_segment(&executable->text);
  copy_segment(&executable->data);
}

void elf_release(struct elf_executable *executable) {
  release_segment(&executable->text);
  release_segment(&executable->data);
}
