TOPDIR=../..
GENSOURCES=GP_Pixel.gen.c GP_Blit.gen.c GP_Convert.gen.c GP_Convert.gen.h
GENHEADERS=GP_Pixel_Scale.gen.h GP_Blit.gen.h GP_Pixel.gen.h GP_GetPutPixel.gen.h
CSOURCES=$(filter-out $(wildcard *.gen.c),$(wildcard *.c))
LIBNAME=core

include $(TOPDIR)/gen.mk
include $(TOPDIR)/include.mk
include $(TOPDIR)/lib.mk
