TOPDIR=../..
GENSOURCES=GP_Text.gen.c
CSOURCES=$(filter-out $(wildcard *.gen.c),$(wildcard *.c))
LIBNAME=text

include $(TOPDIR)/gen.mk
include $(TOPDIR)/include.mk
include $(TOPDIR)/lib.mk

GP_FreeType.dep: CFLAGS+=`freetype-config --cflags`
GP_FreeType.o: CFLAGS+=`freetype-config --cflags`
