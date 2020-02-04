TOPDIR=../..
include $(TOPDIR)/pre.mk

GENSOURCES=GP_Pixel.gen.c GP_Blit.gen.c GP_Convert.gen.c \
           GP_GammaCorrection.gen.c GP_Fill.gen.c

CSOURCES=$(filter-out $(wildcard *.gen.c),$(wildcard *.c))
LIBNAME=core

GP_WritePixel.o: CFLAGS+=-Wimplicit-fallthrough=0

include $(TOPDIR)/gen.mk
include $(TOPDIR)/lib.mk
include $(TOPDIR)/post.mk
