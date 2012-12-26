TOPDIR=../..
include $(TOPDIR)/pre.mk

GENSOURCES=GP_Pixel.gen.c GP_Blit.gen.c GP_Convert.gen.c \
           GP_GammaCorrection.gen.c
CSOURCES=$(filter-out $(wildcard *.gen.c),$(wildcard *.c))
LIBNAME=core

include $(TOPDIR)/gen.mk
include $(TOPDIR)/lib.mk
include $(TOPDIR)/post.mk
