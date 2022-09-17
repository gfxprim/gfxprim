TOPDIR=../..
include $(TOPDIR)/pre.mk

CSOURCES=$(wildcard *.c)

#ifeq ($(wildcard $(os)),$(os))
CSOURCES+=$(wildcard $(os)/*.c)
#endif

INCLUDE=core
LIBNAME=input

gen_lookup:
	./gen_lookup.sh > keys.h

include $(TOPDIR)/lib.mk
include $(TOPDIR)/post.mk
