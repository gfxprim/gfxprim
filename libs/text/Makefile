TOPDIR=../..
CSOURCES=$(shell ls *.c)
LIBNAME=text

include $(TOPDIR)/include.mk
include $(TOPDIR)/lib.mk

GP_FreeType.dep: CFLAGS+=`freetype-config --cflags`
GP_FreeType.o: CFLAGS+=`freetype-config --cflags`
