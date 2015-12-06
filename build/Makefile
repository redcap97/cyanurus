include config.mak

KERNEL     = kernel/cyanurus.elf
INIT       = init/init
DISK_IMAGE = disk.img
MOUNT_DIR  = $(basename $(DISK_IMAGE))
QEMUFLAGS  = -M vexpress-a9 -m 1G -nographic -drive if=sd,file=$(DISK_IMAGE),format=raw

.PHONY: all clean clobber test copy run pty debug-run debug-pty

all: $(DISK_IMAGE) build-kernel build-init build-test-kernel build-test-user

build-kernel:
	$(MAKE) -C kernel

build-musl:
	$(MAKE) -C musl

build-crt: build-musl
	$(MAKE) -C crt

build-init: build-musl build-crt
	$(MAKE) -C init

build-test-kernel: build-init
	$(MAKE) -C test-kernel

build-test-user: build-musl build-crt build-init
	$(MAKE) -C test-user

clean:
	$(MAKE) -C kernel clean
	$(MAKE) -C musl clean
	$(MAKE) -C crt clean
	$(MAKE) -C init clean
	$(MAKE) -C test-kernel clean
	$(MAKE) -C test-user clean

clobber:
	rm -f $(DISK_IMAGE)
	$(MAKE) -C kernel clobber
	$(MAKE) -C musl clobber
	$(MAKE) -C crt clobber
	$(MAKE) -C init clobber
	$(MAKE) -C test-kernel clobber
	$(MAKE) -C test-user clobber

test: all
	$(MAKE) -C test-kernel test
	$(MAKE) -C test-user test

copy: all $(DISK_IMAGE)
	mkdir -p $(MOUNT_DIR)
	sudo mount $(DISK_IMAGE) $(MOUNT_DIR)
	mkdir -p $(MOUNT_DIR)/sbin
	cp $(INIT) $(MOUNT_DIR)/sbin/init
	sudo umount $(MOUNT_DIR)

$(DISK_IMAGE):
	$(QEMU_IMG) create -f raw $@ 64M
	$(MKFS.MINIX) -3 $@

pty:       QEMUFLAGS += -serial pty
debug-run: QEMUFLAGS += -s -S
debug-pty: QEMUFLAGS += -s -S -serial pty

run pty debug-run debug-pty: all
	$(QEMU) $(QEMUFLAGS) -kernel $(KERNEL)
