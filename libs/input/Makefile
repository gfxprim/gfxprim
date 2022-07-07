TOPDIR=../..
include $(TOPDIR)/pre.mk

CSOURCES=$(shell ls *.c)
INCLUDE=core
LIBNAME=input

gen_lookup:
	./gen_lookup.sh > keys.h

include $(TOPDIR)/lib.mk
include $(TOPDIR)/post.mk
