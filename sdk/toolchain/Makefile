# Dreamcast toolchain makefile by Jim Ursetto
# adapted from Stalin's build script version 0.3
# and modified for DreamShell by SWAT and YevDev
#
# Interesting parameters:
# erase=0|1                     Erase build directories on the fly to save space
# thread_model=posix|single|kos Set gcc threading model
# verbose=0|1                   Display
# makejobs=-jn                  Set the number of jobs for calls to make to n
#
# Interesting targets (you can 'make' any of these):
# all: patch build
# patch: patch-gcc patch-newlib
# build: build-sh4 build-arm
# build-sh4: build-sh4-binutils build-sh4-gcc
# build-arm: build-arm-binutils build-arm-gcc
# build-sh4-gcc: build-sh4-gcc-pass1 build-sh4-newlib build-sh4-gcc-pass2
# build-arm-gcc: build-arm-gcc-pass1
# build-sh4-newlib: build-sh4-newlib-only fixup-sh4-newlib
# build-arm-newlib: build-arm-newlib-only fixup-arm-newlib
# gdb
# insight

#
# macOS build intruction:
# $ brew install gcc
# $ export PATH=/usr/local/Cellar/gcc/X.X.X/bin:$PATH
# $ sudo make <target> CC=gcc-X CXX=g++-X CPP=cpp-X LD=g++-X
# where X - gcc version, for instance:
# $ export PATH=/usr/local/Cellar/gcc/8.1.0/bin:$PATH
# $ sudo make all CC=gcc-8 CXX=g++-8 CPP=cpp-8 LD=g++-8
#

# User configuration
sh_target=sh-elf
arm_target=arm-eabi
sh_prefix  := /opt/toolchains/dc/$(sh_target)
arm_prefix := /opt/toolchains/dc/$(arm_target)
# kos_root: KOS Git root (contains kos/ and kos-ports/)
kos_root=/usr/local/dc/kos
# kos_base: equivalent of KOS_BASE (contains include/ and kernel/)
kos_base=/usr/local/dc/kos/kos
binutils_ver=2.34
gcc_ver=9.3.0
newlib_ver=3.3.0
gdb_ver=9.2
insight_ver=6.8-1a

# With GCC 4.x versions, the patches provide a kos thread model, so you should
# use it. With 3.4.6, you probably want posix here. If you really don't want
# threading support for C++ (or Objective C/Objective C++), you can set this to
# single (why you would is beyond me, though).
thread_model=kos
erase=1
verbose=1

# Set this value to -jn where n is the number of jobs you want to run with make.
# If you only want one job, just set this to nothing (i.e, "makejobs=").
# Tracking down problems with multiple make jobs is much more difficult than
# with just one running at a time. So, if you run into trouble, then you should
# clear this variable and try again with just one job running.
makejobs=-j2

# Set the languages to build for pass 2 of building gcc for sh-elf. The default
# here is to build C, C++, Objective C, and Objective C++. You may want to take
# out the latter two if you're not worried about them and/or you're short on
# hard drive space.
pass2_languages=c,c++,objc,obj-c++

# Change this if you don't have Bash installed in /bin
SHELL = /bin/bash

# GCC compiles fine with clang, but only if we use libstdc++ instead of libc++.
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
  ifneq ($(shell $(CXX) --version | grep clang),)
    CXX := "$(CXX) -stdlib=libstdc++ -fbracket-depth=2048"
  endif
endif

# Catch all...
ifeq ($(CXX),)
  CXX := g++
endif


# Makefile variables
install=$(prefix)/bin
pwd := $(shell pwd)
patches := $(pwd)/patches
logdir := $(pwd)/logs
PATH := $(sh_prefix)/bin:$(arm_prefix)/bin:$(PATH)
binutils_dir=binutils-$(binutils_ver)
gcc_dir=gcc-$(gcc_ver)
newlib_dir=newlib-$(newlib_ver)

all: patch build

# ---- patch {{{
binutils_patches := $(wildcard $(patches)/binutils-$(binutils_ver)*.diff)
gcc_patches      := $(wildcard $(patches)/gcc-$(gcc_ver)*.diff)
newlib_patches   := $(wildcard $(patches)/newlib-$(newlib_ver)*.diff)

patch_targets=patch-binutils patch-gcc patch-newlib

patch: $(patch_targets)
patch-binutils: $(binutils_patches)
patch-gcc: $(gcc_patches)
patch-newlib: $(newlib_patches)

$(newlib_patches):
	patch -d $(newlib_dir) -p1 < $@

$(binutils_patches):
	patch -d $(binutils_dir) -p1 < $@

$(gcc_patches):
	patch -d $(gcc_dir) -p1 < $@

# ---- }}}

# ---- build {{{

build: build-sh4 build-arm
build-sh4: build-sh4-binutils build-sh4-gcc
build-arm: build-arm-binutils build-arm-gcc
build-sh4-gcc: build-sh4-gcc-pass1 build-sh4-newlib build-sh4-gcc-pass2
build-arm-gcc: build-arm-gcc-pass1 build-arm-newlib
	$(clean_arm_hack)
build-sh4-newlib: build-sh4-newlib-only fixup-sh4-newlib
build-arm-newlib: build-arm-newlib-only fixup-arm-newlib

# Ensure that, no matter where we enter, prefix and target are set correctly.
build_sh4_targets=build-sh4-binutils build-sh4-gcc build-sh4-gcc-pass1 build-sh4-newlib build-sh4-newlib-only build-sh4-gcc-pass2
build_arm_targets=build-arm-binutils build-arm-gcc build-arm-gcc-pass1 build-arm-newlib build-arm-newlib-only
$(build_sh4_targets): prefix = $(sh_prefix)
$(build_sh4_targets): target = $(sh_target)
$(build_sh4_targets): extra_configure_args = --with-multilib-list=m4-single-only --with-endian=little --with-cpu=m4-single-only
$(build_arm_targets): prefix = $(arm_prefix)
$(build_arm_targets): target = $(arm_target)
$(build_arm_targets): extra_configure_args = --with-arch=armv4

# To avoid code repetition, we use the same commands for both
# architectures.  But we can't create a single target called 
# build-binutils for both sh4 and arm, because phony targets 
# can't be run multiple times.  So we create multiple targets.
build_binutils     = build-sh4-binutils  build-arm-binutils
build_gcc_pass1    = build-sh4-gcc-pass1 build-arm-gcc-pass1
build_newlib_sh4   = build-sh4-newlib-only
build_gcc_pass2    = build-sh4-gcc-pass2
build_newlib_arm   = build-arm-newlib-only

# Here we use the essentially same code for multiple targets,
# differing only by the current state of the variables below.
$(build_binutils): build = build-binutils-$(target)-$(binutils_ver)
$(build_binutils): src_dir = binutils-$(binutils_ver)
$(build_binutils): log = $(logdir)/$(build).log
$(build_binutils): logdir
	@echo "+++ Building $(src_dir) to $(build)..."
	-mkdir -p $(build)
	> $(log)
	cd $(build); ../$(src_dir)/configure --target=$(target) --prefix=$(prefix) --disable-werror CXX=$(CXX) $(to_log)
	make $(makejobs) -C $(build) DESTDIR=$(DESTDIR) $(to_log)
	make -C $(build) install DESTDIR=$(DESTDIR) $(to_log)
	$(clean_up)

$(build_gcc_pass1) $(build_gcc_pass2): build = build-gcc-$(target)-$(gcc_ver)
$(build_gcc_pass1) $(build_gcc_pass2): src_dir = gcc-$(gcc_ver)
$(build_gcc_pass1): log = $(logdir)/$(build)-pass1.log
$(build_gcc_pass1): logdir
	@echo "+++ Building $(src_dir) to $(build) (pass 1)..."
	-mkdir -p $(build)
	> $(log)
	cd $(build);  ../$(src_dir)/configure --target=$(target) --prefix=$(prefix) --without-headers --with-newlib --enable-languages=c --disable-libssp --disable-tls $(extra_configure_args) CXX=$(CXX) $(to_log)
	make $(makejobs) -C $(build) DESTDIR=$(DESTDIR) $(to_log)
	make -C $(build) install DESTDIR=$(DESTDIR) $(to_log)

$(build_newlib_sh4) $(build_newlib_arm): build = build-newlib-$(target)-$(newlib_ver)
$(build_newlib_sh4) $(build_newlib_arm): src_dir = newlib-$(newlib_ver)
$(build_newlib_sh4) $(build_newlib_arm): log = $(logdir)/$(build).log
$(build_newlib_sh4) $(build_newlib_arm): logdir
	@echo "+++ Building $(src_dir) to $(build)..."
	-mkdir -p $(build)
	> $(log)
	cd $(build); ../$(src_dir)/configure --target=$(target) --prefix=$(prefix) $(extra_configure_args) $(to_log)
	make $(makejobs) -C $(build) DESTDIR=$(DESTDIR) $(to_log)
	make -C $(build) install DESTDIR=$(DESTDIR) $(to_log)
	$(clean_up)

fixup-sh4-newlib: newlib_inc=$(DESTDIR)$(sh_prefix)/$(sh_target)/include
fixup-sh4-newlib: $(build_newlib_sh4)
	@echo "+++ Fixing up sh4 newlib includes..."
# KOS pthread.h is modified
# to define _POSIX_THREADS
# pthreads to kthreads mapping
# so KOS includes are available as kos/file.h
# kos/thread.h requires arch/arch.h
# arch/arch.h requires dc/video.h
	cp $(kos_base)/include/pthread.h $(newlib_inc)
	cp $(kos_base)/include/sys/_pthread.h $(newlib_inc)/sys
	cp $(kos_base)/include/sys/sched.h $(newlib_inc)/sys
	ln -nsf $(kos_base)/include/kos $(newlib_inc)
	ln -nsf $(kos_base)/kernel/arch/dreamcast/include/arch $(newlib_inc)
	ln -nsf $(kos_base)/kernel/arch/dreamcast/include/dc   $(newlib_inc)

fixup-arm-newlib: newlib_inc=$(DESTDIR)$(arm_prefix)/$(arm_target)/include
fixup-arm-newlib: $(build_newlib_arm)
	@echo "+++ Fixing up arm newlib includes..."

$(build_gcc_pass2): log = $(logdir)/$(build)-pass2.log
$(build_gcc_pass2): logdir
	@echo "+++ Building $(src_dir) to $(build) (pass 2)..."
	-mkdir -p $(build)
	> $(log)
	cd $(build);  ../$(src_dir)/configure --target=$(target) --prefix=$(prefix) --with-newlib --disable-libssp --disable-tls \
	   --enable-threads=$(thread_model) --enable-languages=$(pass2_languages) $(extra_configure_args) CXX=$(CXX) $(to_log)
	make $(makejobs) -C $(build) DESTDIR=$(DESTDIR) $(to_log)
	make -C $(build) install DESTDIR=$(DESTDIR) $(to_log)
	$(clean_up)

# ---- }}}}


# GDB building

gdb-$(gdb_ver).tar.bz2:
	@echo "+++ Downloading GDB..."
	wget -c ftp://ftp.gnu.org/gnu/gdb/gdb-$(gdb_ver).tar.bz2

unpack_gdb: gdb-$(gdb_ver).tar.bz2 unpack_gdb_stamp

unpack_gdb_stamp: 
	@echo "+++ Unpacking GDB..."
	rm -f $@
	rm -rf gdb-$(gdb_ver)
	tar jxf gdb-$(gdb_ver).tar.bz2
	touch $@

build_gdb: log = $(logdir)/gdb-$(gdb_ver).log
build_gdb: logdir
build_gdb: unpack_gdb build_gdb_stamp

build_gdb_stamp:
	@echo "+++ Building GDB..."
	rm -f $@
	> $(log)
	rm -rf build-gdb-$(gdb_ver)
	mkdir build-gdb-$(gdb_ver)
	cd build-gdb-$(gdb_ver); ../gdb-$(gdb_ver)/configure \
	    --prefix=$(sh_prefix) \
	    --target=$(sh_target) \
		--disable-werror $(to_log)
	make $(makejobs) -C build-gdb-$(gdb_ver) $(to_log)
	touch $@

install_gdb: log = $(logdir)/gdb-$(gdb_ver).log
install_gdb: logdir
install_gdb: build_gdb install_gdb_stamp

install_gdb_stamp:
	@echo "+++ Installing GDB..."
	rm -f $@
	make -C build-gdb-$(gdb_ver) install DESTDIR=$(DESTDIR) $(to_log)
	touch $@

gdb: install_gdb


# INSIGHT building

insight-$(insight_ver).tar.bz2:
	@echo "+++ Downloading INSIGHT..."
	wget -c ftp://sourceware.org/pub/insight/releases/insight-$(insight_ver).tar.bz2

unpack_insight: insight-$(insight_ver).tar.bz2 unpack_insight_stamp

unpack_insight_stamp:
	@echo "+++ Unpacking INSIGHT..."
	rm -f $@
	rm -rf insight-$(insight_ver)
	tar jxf insight-$(insight_ver).tar.bz2
	touch $@

build_insight: log = $(logdir)/insight-$(insight_ver).log
build_insight: logdir
build_insight: unpack_insight build_insight_stamp

build_insight_stamp:
	@echo "+++ Building INSIGHT..."
	rm -f $@
	> $(log)
	rm -rf build-insight-$(insight_ver)
	mkdir build-insight-$(insight_ver)
	cd build-insight-$(insight_ver); ../insight-$(insight_ver)/configure \
	    --prefix=$(sh_prefix) \
	    --target=$(sh_target) \
		--disable-werror $(to_log)
	make $(makejobs) -C build-insight-$(insight_ver) $(to_log)
	touch $@

install_insight: log = $(logdir)/insight-$(insight_ver).log
install_insight: logdir
install_insight: build_insight install_insight_stamp

install_insight_stamp:
	@echo "+++ Installing INSIGHT..."
	rm -f $@
	make -C build-insight-$(insight_ver) install DESTDIR=$(DESTDIR) $(to_log)
	touch $@

insight: install_insight


# ---- support {{{

clean:
	-rm -rf build-newlib-$(sh_target)-$(newlib_ver)
	-rm -rf build-newlib-$(arm_target)-$(newlib_ver)
	-rm -rf build-gcc-$(sh_target)-$(gcc_ver)
	-rm -rf build-gcc-$(arm_target)-$(gcc_ver)
	-rm -rf build-binutils-$(sh_target)-$(binutils_ver)
	-rm -rf build-binutils-$(arm_target)-$(binutils_ver)
	-rm -rf build-gdb-$(gdb_ver) install_gdb_stamp build_gdb_stamp 
	-rm -rf build-insight-$(gdb_ver) install_insight_stamp build_insight_stamp

logdir:
	@mkdir -p $(logdir)

# If erase=1, erase build directories on the fly.
ifeq (1,$(erase))
  define clean_up
    @echo "+++ Cleaning up $(build)..."
    -rm -rf $(build)
  endef
  # Hack to clean up ARM gcc pass 1
  define clean_arm_hack
    @echo "+++ Cleaning up build-gcc-$(arm_target)-$(gcc_ver)..."
    -rm -rf build-gcc-$(arm_target)-$(gcc_ver)
  endef
endif

# If verbose=1, display output to screen as well as log files
ifeq (1,$(verbose))
  to_log = 2>&1 | tee -a $(log) && [ $$PIPESTATUS -eq 0 ]
else
  to_log = >> $(log) 2>&1
endif

# ---- }}}

# ---- phony targets {{{

.PHONY: $(patch_targets)
.PHONY: $(newlib_patches) $(binutils_patches) $(gcc_patches) $(kos_patches)
.PHONY: all build patch build-sh4 build-arm $(build_sh4_targets) $(build_arm_targets) clean
.PHONY: build-binutils build-newlib build-gcc-pass1 build-gcc-pass2 fixup-sh4-newlib
.PHONY: gdb install_gdb build_gdb unpack_gdb
.PHONY: insight install_insight build_insight unpack_insight

# ---- }}}}

# vim:tw=0:fdm=marker:fdc=2:fdl=1
