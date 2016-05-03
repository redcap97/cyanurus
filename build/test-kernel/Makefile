ROOT_DIR  = ../..
BUILD_DIR = ..

include $(BUILD_DIR)/config.mak
include $(BUILD_DIR)/kernel/objs.mak
include objs.mak

TEST_DIR = $(ROOT_DIR)/src/test-kernel
SRC_DIR  = $(ROOT_DIR)/src/kernel
VPATH = $(TEST_DIR) $(SRC_DIR)

COOP = $(ROOT_DIR)/tool/coop/bin/coop --source $(TEST_DIR)
GEN_CONFIG = $(ROOT_DIR)/tool/gen_config

TARGET = cyanurus
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

all: $(TARGET).elf

clean:
	rm -f entries.h config.h $(OBJS) $(DEPS) $(TARGET).elf

clobber: clean

test: all
	$(COOP) run

$(TARGET).elf: $(OBJS)
	$(ARCH)-gcc $(OBJS) -o $(TARGET).elf $(LDFLAGS)

-include $(DEPS)

$(OBJS): config.h

config.h: $(GEN_CONFIG)
	$(GEN_CONFIG) > $@

kernel.o: entries.h

entries.h: $(shell $(COOP) list_entry_files)
	$(COOP) dump_entries > $@
