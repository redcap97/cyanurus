ROOT_DIR  = ../..
BUILD_DIR = ..

include $(BUILD_DIR)/config.mak

MUSL_CFLAGS = -Os -march=armv7-a -mfloat-abi=soft -marm
MUSL_OPTION = --disable-shared

SRC_DIR = $(ROOT_DIR)/src/musl
VPATH = $(SRC_DIR)

.PHONY: all clean clobber

all: lib/libc.a

clean:

clobber: clean
	$(MAKE) -C $(SRC_DIR) clean
	rm -f $(SRC_DIR)/config.mak

lib/libc.a: MAKEOVERRIDES=
lib/libc.a: config.mak
	$(MAKE) -C $(SRC_DIR)

config.mak: VERSION
	cd $(SRC_DIR); CROSS_COMPILE="$(ARCH)-" CFLAGS="$(MUSL_CFLAGS)" ./configure $(MUSL_OPTION)
