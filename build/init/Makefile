ROOT_DIR  = ../..
BUILD_DIR = ..

include $(BUILD_DIR)/config.mak
include objs.mak

SRC_DIR = $(ROOT_DIR)/src/init
MUSL_DIR = $(ROOT_DIR)/src/musl
VPATH = $(SRC_DIR)

TARGET = init
CRT0 = $(BUILD_DIR)/crt/crt0.o
LIBC = $(BUILD_DIR)/musl/lib/libc.a
SUBDIRS = $(sort $(dir $(OBJS)))
DEPS = $(OBJS:%.o=%.d)

export C_INCLUDE_PATH = $(MUSL_DIR)/include
LDFLAGS += -T $(BUILD_DIR)/ldscript/user.ld -L $(MUSL_DIR)/lib -lc -lgcc
CFLAGS  += -I$(SRC_DIR) -I.

create-subdirs :=                   \
  $(shell for dir in $(SUBDIRS);    \
    do                              \
      test -d $$dir || mkdir $$dir; \
    done)

.PHONY: all clean clobber

all: $(TARGET)

clean:
	rm -f $(OBJS) $(DEPS) $(TARGET)

clobber: clean

$(TARGET): $(OBJS) $(CRT0) $(LIBC)
	$(ARCH)-gcc $(OBJS) $(CRT0) -o $(TARGET) $(LDFLAGS)

-include $(DEPS)
