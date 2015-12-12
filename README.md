# Cyanurus

Cyanurus is a Unix-like operating system for ARMv7-A.

![Screen Capture](https://cloud.githubusercontent.com/assets/928237/11760605/6e2ad110-a0e4-11e5-861e-8686bd7f3cc1.gif)

## How to Run

Cyanurus kernel and rootfs image are available at below link:

https://github.com/redcap97/cyanurus/releases/download/v0.1.0/cyanurus-0.1.0.tar.xz

QEMU is required to run Cyanurus. Please type following command:

```
qemu-system-arm -M vexpress-a9 -m 1G -nographic -drive if=sd,file=rootfs.img,format=raw -kernel cyanurus.elf
```

Cyanurus is also able to run on docker containers. Please type following command:

```
docker run -it --rm redcap97/cyanurus
```

## Build Dependencies

### Required

* Linux
* gmake
* gcc (arm-none-eabi)
* binutils (arm-none-eabi)
* ruby
* qemu-system-arm
* qemu-img
* mkfs.minix

### Optional

* clang
* gdb (arm-none-eabi)

## License

Licensed under the Apache License v2.0.
