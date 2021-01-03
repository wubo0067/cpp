.PHONY: modules modules_install clean

obj-m += km-calmdev.o
CURRENT_PATH := $(shell pwd)
LINUX_KERNEL := $(shell uname -r)
LINUX_KERNEL_PATH := /usr/src/kernels/$(LINUX_KERNEL)
all:
	make -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) modules EXTRA_CFLAGS="-g -DDEBUG"
clean:
	make -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) clean