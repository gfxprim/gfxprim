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
