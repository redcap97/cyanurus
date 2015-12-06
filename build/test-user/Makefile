ROOT_DIR  = ../..
BUILD_DIR = ..

include $(BUILD_DIR)/config.mak
include $(BUILD_DIR)/kernel/objs.mak
include tests.mak

TEST_DIR = $(ROOT_DIR)/src/test-user
SRC_DIR  = $(ROOT_DIR)/src/kernel
VPATH = $(TEST_DIR) $(SRC_DIR)

COOP = $(ROOT_DIR)/tool/coop/bin/coop -s $(TEST_DIR)
CYANURUS_CC = $(ROOT_DIR)/tool/cc
GEN_CONFIG = $(ROOT_DIR)/tool/gen_config

TARGET = cyanurus
CRT0 = $(BUILD_DIR)/crt/crt0.o
LIBC = $(BUILD_DIR)/musl/lib/libc.a
SUBDIRS = $(sort $(dir $(OBJS)))
DEPS = $(OBJS:%.o=%.d)

CFLAGS  += -I$(SRC_DIR) -I$(BUILD_DIR) -I$(TEST_DIR) -I.
ASFLAGS += -I$(SRC_DIR) -I$(BUILD_DIR) -I$(TEST_DIR) -I.
LDFLAGS += -T $(BUILD_DIR)/ldscript/kernel.ld -lgcc

create-subdirs :=                   \
  $(shell for dir in $(SUBDIRS);    \
    do                              \
      test -d $$dir || mkdir $$dir; \
    done)

.PHONY: all clean clobber test

all: $(TARGET).elf $(TESTS)

clean:
	rm -f config.h $(OBJS) $(DEPS) $(TARGET).elf $(TESTS)

clobber: clean

test: all
	$(COOP) run

$(TARGET).elf: $(OBJS)
	$(ARCH)-gcc $(OBJS) -o $(TARGET).elf $(LDFLAGS)

$(TESTS): $(CRT0) $(LIBC)

%: %.c
	$(CYANURUS_CC) -I $(TEST_DIR) -Os -std=gnu99 -Wall -Wextra -Werror -o $@ $<

-include $(DEPS)

$(OBJS): config.h

config.h: $(GEN_CONFIG)
	$(GEN_CONFIG) > $@
