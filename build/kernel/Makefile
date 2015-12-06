ROOT_DIR  = ../..
BUILD_DIR = ..

include $(BUILD_DIR)/config.mak
include objs.mak

SRC_DIR = $(ROOT_DIR)/src/kernel
VPATH = $(SRC_DIR)

GEN_CONFIG = $(ROOT_DIR)/tool/gen_config

TARGET = cyanurus
SUBDIRS = $(sort $(dir $(OBJS)))
DEPS = $(OBJS:%.o=%.d)

CFLAGS  += -I$(SRC_DIR) -I$(BUILD_DIR) -I.
ASFLAGS += -I$(SRC_DIR) -I$(BUILD_DIR) -I.
LDFLAGS += -T $(BUILD_DIR)/ldscript/kernel.ld -lgcc

create-subdirs :=                   \
  $(shell for dir in $(SUBDIRS);    \
    do                              \
      test -d $$dir || mkdir $$dir; \
    done)

.PHONY: all clean clobber

all: $(TARGET).elf

clean:
	rm -f config.h $(OBJS) $(DEPS) $(TARGET).elf

clobber: clean

$(TARGET).elf: $(OBJS)
	$(ARCH)-gcc $(OBJS) -o $(TARGET).elf $(LDFLAGS)

-include $(DEPS)

$(OBJS): config.h

config.h: $(GEN_CONFIG)
	$(GEN_CONFIG) > $@
