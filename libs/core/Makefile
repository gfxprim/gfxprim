TOPDIR=../..
GENSOURCES=GP_Pixel.gen.c GP_Blit.gen.c
GENHEADERS=GP_Pixel_Scale.gen.h GP_Blit.gen.h GP_Pixel.gen.h
CSOURCES=$(filter-out $(wildcard *.gen.c),$(wildcard *.c))
LIBNAME=core

include $(TOPDIR)/gen.mk
include $(TOPDIR)/include.mk
include $(TOPDIR)/lib.mk

