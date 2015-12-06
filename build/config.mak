ARCH = arm-none-eabi

QEMU     = qemu-system-arm
QEMU_IMG = qemu-img

MKFS.MINIX = mkfs.minix

ifdef USE_CLANG
	CC = clang -integrated-as
	AS = clang -integrated-as

	CFLAGS_ARCH  = -target armv7a-eabi -mfloat-abi=soft -marm -fshort-enums
	ASFLAGS_ARCH = -target armv7a-eabi -mfloat-abi=hard -marm -fshort-enums
else
	CC = $(ARCH)-gcc
	AS = $(CC)

	CFLAGS_ARCH  = -march=armv7-a -mfloat-abi=soft -marm
	ASFLAGS_ARCH = $(CFLAGS_ARCH)
endif

CFLAGS  = -Os -std=gnu99 -nostdinc -nostdlib -fno-builtin -Wall -Wextra -Werror
ASFLAGS = $(CFLAGS)

LDFLAGS = -static -nostdlib

# for gdb
ifdef DEBUG
	CFLAGS += -g
endif

%.o: %.c
	$(CC) $(CFLAGS) $(CFLAGS_ARCH) -c -MD -MP -o $@ $<

%.o: %.S
	$(AS) $(ASFLAGS) $(ASFLAGS_ARCH) -c -MD -MP -o $@ $<
