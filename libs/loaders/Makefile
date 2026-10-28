TOPDIR=../..
include $(TOPDIR)/pre.mk

CSOURCES=$(shell ls *.c)
INCLUDE=core
LIBNAME=loaders
BUILDLIB=yes

gp_bmp.o: CFLAGS+=$(CFLAGS_WNIF)
gp_jp2.dep: CFLAGS+=$(OPENJPEG_CFLAGS)
gp_jp2.o: CFLAGS+=$(OPENJPEG_CFLAGS)

include $(TOPDIR)/lib.mk
include $(TOPDIR)/post.mk
