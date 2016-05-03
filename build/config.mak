ARCH = arm-none-eabi

ifdef USE_CLANG
	CC = clang -integrated-as
	AS = clang -integrated-as

	CFLAGS_ARCH  = -target armv7a-eabi -mfloat-abi=soft -marm -fshort-enums
	ASFLAGS_ARCH = -target armv7a-eabi
else
	CC = $(ARCH)-gcc
	AS = $(CC)

	CFLAGS_ARCH  = -march=armv7-a -mfloat-abi=soft -marm
	ASFLAGS_ARCH = -march=armv7-a
endif

CFLAGS  = -Os -Wall -Wextra -Werror -std=gnu99 -nostdinc -nostdlib -fno-builtin
ASFLAGS = -Os -Wall -Wextra -Werror

LDFLAGS = -static -nostdlib

# for gdb
ifdef DEBUG
	CFLAGS += -g
endif

%.o: %.c
	$(CC) $(CFLAGS) $(CFLAGS_ARCH) -c -MD -MP -o $@ $<

%.o: %.s
	$(AS) $(ASFLAGS) $(ASFLAGS_ARCH) -c -o $@ $<
