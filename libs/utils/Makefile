TOPDIR=../..
include $(TOPDIR)/pre.mk

CSOURCES=$(filter-out $(wildcard *.gen.c),$(wildcard *.c))
LIBNAME=core

#ifeq ($(wildcard $(os)),$(os))
CSOURCES+=$(wildcard $(os)/*.c)
#endif

include $(TOPDIR)/lib.mk
include $(TOPDIR)/post.mk
