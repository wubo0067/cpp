OUTPUT := .output
# :前面的变量不能使用后面的变量，只能使用前面已定义好了的变量
SRCS			= $(wildcard *.c)
BPF_KERN_OBJS  := $(patsubst %_kern.c, %_kern.o, $(SRCS))

BPFTOOL ?= /usr/sbin/bpftool
CFLAGS := -g -O2 -Wall
INCLUDES =
LLC = llc
CC = gcc
CLANG = clang
OPT = opt
LLVM_DIS ?= llvm-dis
KERNEL_SRC = /usr/src/linux-5.12.9
ARCH := $(shell uname -m | sed 's/x86_64/x86/' | sed 's/aarch64/arm64/' | sed 's/ppc64le/powerpc/' | sed 's/mips.*/mips/')
KERNEL_VERSION="$(shell cat $(KERNEL_SRC)/include/config/kernel.release)"
VER_MAJOR=$(shell echo $(KERNEL_VERSION) | cut -d. -f1)
VER_MINOR=$(shell echo $(KERNEL_VERSION) | cut -d. -f2)
VER_PATCH=$(shell echo $(KERNEL_VERSION) | cut -d. -f3)
CURRENT_KERNEL=$(shell echo $(VER_MAJOR)\*65536 + $(VER_MINOR)\*256 + $(VER_PATCH) |bc)

LIBS = $(KERNEL_SRC)/tools/lib/bpf/libbpf.a -lelf -lz

KERNEL_SRC_INCLUDE    := \
		-I$(KERNEL_SRC)/arch/$(ARCH)/include \
		-I$(KERNEL_SRC)/arch/$(ARCH)/include/generated \
		-I$(KERNEL_SRC)/include \
		-I$(KERNEL_SRC)/arch/$(ARCH)/include/uapi \
		-I$(KERNEL_SRC)/arch/$(ARCH)/include/generated/uapi \
		-I$(KERNEL_SRC)/include/uapi \
		-I$(KERNEL_SRC)/include/generated/uapi \
		-I$(KERNEL_SRC)/samples/bpf \
		-I$(KERNEL_SRC)/tools/testing/selftests/bpf \
		-I$(KERNEL_SRC)/tools/lib \
		-include $(KERNEL_SRC)/include/linux/kconfig.h
		# -include $(KERNEL_SRC)/include/linux/compiler-version.h 
		

K_CFLAGS = -g -O2 -Wall -nostdinc -isystem `$(CLANG) -print-file-name=include`
K_CFLAGS += -D__KERNEL__ -D__BPF_TRACING__ -D__TARGET_ARCH_$(ARCH)
K_CFLAGS += -Wunused -Wall \
			-Wno-compare-distinct-pointer-types \
			-Wno-pointer-sign \
			-Wno-gnu-variable-sized-type-not-at-end \
			-Wno-address-of-packed-member \
			-Wno-tautological-compare \
			-Wno-unknown-warning-option \
			-fno-stack-protector 

K_CFLAGS += -I./ $(KERNEL_SRC_INCLUDE)

ifeq ("$(origin V)", "command line")
  VERBOSE = $(V)
endif
ifndef VERBOSE
  VERBOSE = 0
endif

ifeq ($(VERBOSE),1)
  Q =
  msg =
else
  Q = @
  msg = @printf '  %-8s %s%s\n' "$(1)" "$(notdir $(2))" "$(if $(3), $(3))";
endif

all: $(BPF_KERN_OBJS)

$(OUTPUT):
	$(call msg,MKDIR,$@)
	$(Q)mkdir -p $@

%_kern.o: %_kern.c
	$(call msg,BPF,$@)
	$(Q)$(CLANG) $(K_CFLAGS) -emit-llvm -Xclang -disable-llvm-passes -c $<  -o - | \
		$(OPT) -O2 -mtriple=bpf-pc-linux | $(LLVM_DIS) | \
		$(LLC) -march=bpf $(LLC_FLAGS) -filetype=obj -o $@	
	/bin/bash rename_binaries.sh "$(VER_MAJOR)" "$(VER_MINOR)" "$@"

.PHONY: clean
clean: 
	$(call msg, CLEANING)
	$(Q)rm -rf $(OUTPUT) $(BPF_KERN_OBJS) *.o
		