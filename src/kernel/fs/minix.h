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

#ifndef _CYANURUS_FS_MINIX_H_
#define _CYANURUS_FS_MINIX_H_

#include "lib/type.h"

#define NAME_MAX 60

struct minix3_superblock {
  uint32_t s_ninodes;
  uint16_t s_pad0;
  uint16_t s_imap_blocks;
  uint16_t s_zmap_blocks;
  uint16_t s_firstdatazone;
  uint16_t s_log_zone_size;
  uint16_t s_pad1;
  uint32_t s_max_size;
  uint32_t s_zones;
  uint16_t s_magic;
  uint16_t s_pad2;
  uint16_t s_blocksize;
  uint8_t  s_disk_version;
};

struct minix2_inode {
  uint16_t i_mode;
  uint16_t i_nlinks;
  uint16_t i_uid;
  uint16_t i_gid;
  uint32_t i_size;
  uint32_t i_atime;
  uint32_t i_mtime;
  uint32_t i_ctime;
  uint32_t i_zone[10];
};

struct minix3_dirent {
  uint32_t d_ino;
  char d_name[NAME_MAX];
};

#endif
