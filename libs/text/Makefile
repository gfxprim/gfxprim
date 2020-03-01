TOPDIR=../..
include $(TOPDIR)/pre.mk
include $(TOPDIR)/config.mk

GENSOURCES=gp_text.gen.c
CSOURCES=$(filter-out $(wildcard *.gen.c),$(wildcard *.c))
LIBNAME=text

include $(TOPDIR)/gen.mk
include $(TOPDIR)/lib.mk
include $(TOPDIR)/post.mk

ifeq ($(HAVE_FREETYPE),yes)
gp_free_type.dep: CFLAGS+=`freetype-config --cflags`
gp_free_type.o: CFLAGS+=`freetype-config --cflags`
endif
