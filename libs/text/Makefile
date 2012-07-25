TOPDIR=../..
include $(TOPDIR)/pre.mk
include $(TOPDIR)/config.mk

GENSOURCES=GP_Text.gen.c
CSOURCES=$(filter-out $(wildcard *.gen.c),$(wildcard *.c))
LIBNAME=text

include $(TOPDIR)/gen.mk
include $(TOPDIR)/lib.mk
include $(TOPDIR)/post.mk

ifeq ($(HAVE_FREETYPE),yes)
GP_FreeType.dep: CFLAGS+=`freetype-config --cflags`
GP_FreeType.o: CFLAGS+=`freetype-config --cflags`
endif
