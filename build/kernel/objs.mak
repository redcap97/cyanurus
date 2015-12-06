OBJS  = uart.o gic.o mmu.o process.o
OBJS += system.o syscall.o elf.o timer.o mmc.o kernel.o
OBJS += logger.o buddy.o slab.o page.o aeabi.o fs.o tty.o
OBJS += lib/stdarg.o lib/string.o lib/libgen.o lib/list.o
OBJS += lib/setjmp.o lib/signal.o lib/bitset.o
OBJS += fs/block.o fs/inode.o fs/dentry.o fs/superblock.o
OBJS += pipe.o
OBJS += asm/mmu.o asm/system.o asm/vectors.o
