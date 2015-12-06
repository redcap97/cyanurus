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

#include "lib/type.h"
#include "uart.h"
#include "mmc.h"

#define MMC_BASE ((volatile uint8_t*)0x10005000)

#define MMC_POWER       0x00
#define MMC_CLOCK       0x04
#define MMC_ARGUMENT    0x08
#define MMC_COMMAND     0x0c
#define MMC_RESPONSE0   0x14
#define MMC_RESPONSE1   0x18
#define MMC_RESPONSE2   0x1c
#define MMC_RESPONSE3   0x20
#define MMC_DATA_TIMER  0x24
#define MMC_DATA_LENGTH 0x28
#define MMC_DATA_CTRL   0x2c
#define MMC_STATUS      0x34
#define MMC_CLEAR       0x38
#define MMC_MASK0       0x3c
#define MMC_MASK1       0x40
#define MMC_FIFO_CNT    0x48
#define MMC_FIFO        0x80

#define MMC_CMD_GO_IDLE_STATE       0
#define MMC_CMD_ALL_SEND_CID        2
#define MMC_CMD_SEND_RELATIVE_ADDR  3
#define MMC_CMD_SELECT_CARD         7
#define MMC_CMD_SEND_IF_COND        8
#define MMC_CMD_SEND_CSD            9
#define MMC_CMD_STATUS              13
#define MMC_CMD_READ_SINGLE_BLOCK   17
#define MMC_CMD_WRITE_SINGLE_BLOCK  24
#define MMC_CMD_SD_APP_OP_COND      41
#define MMC_CMD_APP_CMD             55

#define MMC_CMD_RESPONSE (1 << 6)
#define MMC_CMD_LONG_RSP (1 << 7)
#define MMC_CMD_ENABLE   (1 << 10)

#define MMC_RCA_MASK     (~((1 << 16) - 1))
#define MMC_FIFO_SIZE    16

#define MMC_BLOCK_SIZE   0x200

struct mmc_cmd_resp {
  uint32_t status;
  uint32_t mask;
  uint32_t resp[4];
};

static void mmc_send_command(uint32_t cmd, uint32_t arg, struct mmc_cmd_resp *p_cresp) {
  *((volatile uint32_t*)(MMC_BASE + MMC_ARGUMENT)) = arg;
  *((volatile uint32_t*)(MMC_BASE + MMC_COMMAND)) = cmd;

  p_cresp->status = *((volatile uint32_t*)(MMC_BASE + MMC_STATUS));
  p_cresp->mask   = *((volatile uint32_t*)(MMC_BASE + MMC_MASK0));

  *((volatile uint32_t*)(MMC_BASE + MMC_CLEAR)) = 0xfff;

  if (cmd & MMC_CMD_RESPONSE) {
    p_cresp->resp[0] = *((volatile uint32_t*)(MMC_BASE + MMC_RESPONSE0));
    p_cresp->resp[1] = *((volatile uint32_t*)(MMC_BASE + MMC_RESPONSE1));
    p_cresp->resp[2] = *((volatile uint32_t*)(MMC_BASE + MMC_RESPONSE2));
    p_cresp->resp[3] = *((volatile uint32_t*)(MMC_BASE + MMC_RESPONSE3));
  }
}

static int mmc_write_block(uint64_t address, const void *data) {
  int i, j;
  const uint32_t *buf = (const uint32_t*)data;
  struct mmc_cmd_resp cresp;

  *((volatile uint32_t*)(MMC_BASE + MMC_DATA_TIMER))  = 0xa000;
  *((volatile uint32_t*)(MMC_BASE + MMC_DATA_LENGTH)) = 0x200;
  *((volatile uint32_t*)(MMC_BASE + MMC_DATA_CTRL))   = 0x91;

  mmc_send_command(MMC_CMD_WRITE_SINGLE_BLOCK | MMC_CMD_RESPONSE | MMC_CMD_ENABLE, address, &cresp);

  for (i = 0; i < ((MMC_BLOCK_SIZE / (int)sizeof(uint32_t)) / MMC_FIFO_SIZE); ++i) {
    for (j = 0; j < MMC_FIFO_SIZE; ++j) {
      *((volatile uint32_t*)(MMC_BASE + MMC_FIFO)) = *buf++;
    }
  }

  return 0;
}

int mmc_write(uint64_t address, size_t size, const void *data) {
  size_t i;
  const uint8_t *buf = (const uint8_t*)data;

  if (size % MMC_BLOCK_SIZE) {
    return -1;
  }

  for (i = 0; i < (size / MMC_BLOCK_SIZE); ++i) {
    if (mmc_write_block(address, buf) < 0) {
      return -1;
    }

    address += MMC_BLOCK_SIZE;
    buf += MMC_BLOCK_SIZE;
  }

  return 0;
}

static int mmc_read_block(uint64_t address, void *data) {
  int i, j;
  uint32_t *buf = (uint32_t*)data, status;
  struct mmc_cmd_resp cresp;

  *((volatile uint32_t*)(MMC_BASE + MMC_DATA_TIMER))  = 0xa000;
  *((volatile uint32_t*)(MMC_BASE + MMC_DATA_LENGTH)) = 0x200;
  *((volatile uint32_t*)(MMC_BASE + MMC_DATA_CTRL))   = 0x93;

  mmc_send_command(MMC_CMD_READ_SINGLE_BLOCK | MMC_CMD_RESPONSE | MMC_CMD_ENABLE, address, &cresp);

  for (i = 0; i < ((MMC_BLOCK_SIZE / (int)sizeof(uint32_t)) / MMC_FIFO_SIZE); ++i) {
    status = *((volatile uint32_t*)(MMC_BASE + MMC_STATUS));
    (void)status;

    for (j = 0; j < MMC_FIFO_SIZE; ++j) {
      *buf++ = *((volatile uint32_t*)(MMC_BASE + MMC_FIFO));
    }
  }

  return 0;
}

int mmc_read(uint64_t address, size_t size, void *data) {
  size_t i;
  uint8_t *buf = (uint8_t*)data;

  if (size % MMC_BLOCK_SIZE) {
    return -1;
  }

  for (i = 0; i < (size / MMC_BLOCK_SIZE); ++i) {
    if (mmc_read_block(address, buf) < 0) {
      return -1;
    }

    address += MMC_BLOCK_SIZE;
    buf += MMC_BLOCK_SIZE;
  }

  return 0;
}

void mmc_init(void) {
  uint32_t rca;
  struct mmc_cmd_resp cresp;

  *((volatile uint32_t*)(MMC_BASE + MMC_CLEAR)) = 0xfff;
  *((volatile uint32_t*)(MMC_BASE + MMC_MASK0)) = 0x2ff;

  *((volatile uint32_t*)(MMC_BASE + MMC_POWER)) = 0x2;
  *((volatile uint32_t*)(MMC_BASE + MMC_CLOCK)) = 0x11d;
  *((volatile uint32_t*)(MMC_BASE + MMC_POWER)) = 0x3;

  mmc_send_command(MMC_CMD_GO_IDLE_STATE | MMC_CMD_ENABLE, 0x00, &cresp);
  mmc_send_command(MMC_CMD_SEND_IF_COND | MMC_CMD_RESPONSE | MMC_CMD_ENABLE, 0x1aa, &cresp);
  mmc_send_command(MMC_CMD_APP_CMD | MMC_CMD_ENABLE, 0x00, &cresp);
  mmc_send_command(MMC_CMD_SD_APP_OP_COND | MMC_CMD_ENABLE, 0x40300000, &cresp);
  mmc_send_command(MMC_CMD_ALL_SEND_CID | MMC_CMD_RESPONSE | MMC_CMD_LONG_RSP | MMC_CMD_ENABLE, 0x00, &cresp);

  mmc_send_command(MMC_CMD_SEND_RELATIVE_ADDR | MMC_CMD_RESPONSE | MMC_CMD_ENABLE, 0x00, &cresp);
  rca = cresp.resp[0] & MMC_RCA_MASK;

  mmc_send_command(MMC_CMD_SEND_CSD | MMC_CMD_RESPONSE | MMC_CMD_LONG_RSP | MMC_CMD_ENABLE, rca, &cresp);
  mmc_send_command(MMC_CMD_SELECT_CARD | MMC_CMD_RESPONSE | MMC_CMD_ENABLE, rca, &cresp);
}
