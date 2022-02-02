OUTPUT := .output
CLANG ?= clang
LLVM_STRIP	?= llvm-strip
BPFTOOL ?= $(abspath ../../../extra/libbpf/tools/bpftool)

SRCS			= $(wildcard *.c)
BPF_KERN_OBJS  := $(patsubst %.bpf.c, %.bpf.o, $(SRCS))

LIBBPF_SRC := $(abspath ../../../extra/libbpf/libbpf/src)
LIBBPF_OBJ := $(abspath ../../../extra/libbpf/libbpf.a)

BPFTOOL_OUTPUT := $(abspath $(OUTPUT)/bpftool)

CLANG_BPF_SYS_INCLUDES = $(shell $(CLANG) -v -E - </dev/null 2>&1 \
	| sed -n '/<...> search starts here:/,/End of search list./{ s| \(/.*\)|-idirafter \1|p }')

INCLUDES := -I$(abspath $(OUTPUT)) \
			-I$(abspath ../../../extra/libbpf) \
			-I$(abspath ../../../extra/libbpf/libbpf/include/uapi)

CFLAGS := -g -Wall

ARCH := $(shell uname -m | sed 's/x86_64/x86/' | sed 's/aarch64/arm64/' | sed 's/ppc64le/powerpc/' | sed 's/mips.*/mips/')

# Try to detect best kernel BTF source
KERNEL_REL := $(shell uname -r)
VMLINUX_BTF_PATHS := /sys/kernel/btf/vmlinux /boot/vmlinux-$(KERNEL_REL)
VMLINUX_BTF_PATH := $(or $(VMLINUX_BTF),$(firstword			       \
					  $(wildcard $(VMLINUX_BTF_PATHS))))

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
  QUIET_CC       = @echo '  CC       '$@;
  QUIET_CC_FPIC  = @echo '  CC FPIC  '$@;
  QUIET_CLANG    = @echo '  CLANG    '$@;
  QUIET_AR       = @echo '  AR       '$@;
  QUIET_LINK     = @echo '  LINK     '$@;
  QUIET_MKDIR    = @echo '  MKDIR    '$@;
  QUIET_GEN      = @echo '  GEN      '$@;
  QUIET_FLEX     = @echo '  FLEX     '$@;
  QUIET_BISON    = @echo '  BISON    '$@;
  QUIET_GENSKEL  = @echo '  GEN-SKEL '$@;
  QUIET_CLEAN    = @printf '  CLEAN    %s\n' $1;
  QUIET_INSTALL  = @printf '  INSTALL  %s\n' $1;
  QUIET_UNINST   = @printf '  UNINST   %s\n' $1; 
  submake_extras := feature_display=0 
endif

.PHONY: all clean
all: $(BPF_KERN_OBJS)

$(BPF_KERN_OBJS): %.bpf.o: $(OUTPUT)/vmlinux.h $(LIBBPF_OBJ) %.bpf.c
	$(call msg,BPF,$@)
	$(Q)$(CLANG) -g -O2 -target bpf -D__TARGET_ARCH_$(ARCH) $(INCLUDES) $(CLANG_BPF_SYS_INCLUDES) -c $(filter %.bpf.c,$^) -o $@	
	$(Q)$(LLVM_STRIP) -g $@ # strip useless DWARF info
	# /bin/bash rename_binaries.sh "$(VER_MAJOR)" "$(VER_MINOR)" "$@"	

$(OUTPUT)/vmlinux.h: $(VMLINUX_BTF_PATH) | $(OUTPUT)
	$(Q)if [ ! -e "$(VMLINUX_BTF_PATH)" ] ; then \
		echo "Couldn't find kernel BTF; set VMLINUX_BTF to"	       \
			"specify its location." >&2;			       \
		exit 1;\
	fi
	$(QUIET_GEN)$(BPFTOOL) btf dump file $(VMLINUX_BTF_PATH) format c > $@

$(LIBBPF_OBJ): $(wildcard $(LIBBPF_SRC)/*.[ch] $(LIBBPF_SRC)/Makefile)
	$(call msg,LIB,$@)
	$(Q)$(MAKE) $(submake_extras) -C $(LIBBPF_SRC) BUILD_STATIC_ONLY=1  \
									OBJDIR=$(dir $@)/lib DESTDIR=$(dir $@)	\
									INCLUDEDIR= LIBDIR= UAPIDIR=	    \
									install

$(OUTPUT):
	$(call msg,MKDIR,$@)
	$(Q)mkdir -p $@

clean:
	$(call msg,CLEAN)
	$(Q)rm -rf $(OUTPUT) $(BPF_KERN_OBJS) $(LIBBPF_OBJ)
