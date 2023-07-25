TOPDIR=../..
include $(TOPDIR)/pre.mk

CSOURCES=$(shell ls *.c)

#ifeq ($(wildcard $(os)),$(os))
CSOURCES+=$(wildcard $(os)/*.c)
#endif

LIBNAME=backends
BUILDLIB=yes

include $(TOPDIR)/lib.mk
include $(TOPDIR)/post.mk

ifeq ($(HAVE_WAYLAND),yes)
gp_backend_wayland.dep: CFLAGS+=$(WAYLAND_CFLAGS)
gp_backend_wayland.o: CFLAGS+=$(WAYLAND_CFLAGS)
endif
