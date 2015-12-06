ROOT_DIR  = ../..
BUILD_DIR = ..

include $(BUILD_DIR)/config.mak
include objs.mak

SRC_DIR = $(ROOT_DIR)/src/crt
MUSL_DIR = $(ROOT_DIR)/src/musl
VPATH = $(SRC_DIR)

TARGET = crt0.o
SUBDIRS = $(sort $(dir $(OBJS)))
DEPS = $(OBJS:%.o=%.d)

export C_INCLUDE_PATH = $(MUSL_DIR)/include
CFLAGS  += -I$(SRC_DIR) -I.

create-subdirs :=                   \
  $(shell for dir in $(SUBDIRS);    \
    do                              \
      test -d $$dir || mkdir $$dir; \
    done)

.PHONY: all clean clobber

all: $(TARGET)

$(TARGET): $(OBJS)
	$(ARCH)-ld -r -o $@ $(OBJS)

clean:
	rm -f $(OBJS) $(DEPS) $(TARGET)

clobber: clean

-include $(DEPS)
