TOPDIR=../..
include $(TOPDIR)/pre.mk

CSOURCES=$(shell ls *.c)
INCLUDE=core
LIBNAME=loaders
BUILDLIB=yes

GP_BMP.o: CFLAGS+=$(CFLAGS_WNIF)

include $(TOPDIR)/lib.mk
include $(TOPDIR)/post.mk
