TOPDIR=../..
GENSOURCES=GP_Brightness.gen.c GP_Contrast.gen.c GP_Invert.gen.c\
           GP_MirrorV.gen.c GP_Rotate.gen.c GP_Dither.gen.c
GENHEADERS=
CSOURCES=$(filter-out $(wildcard *.gen.c),$(wildcard *.c))
LIBNAME=filters
INCLUDE=core

include $(TOPDIR)/gen.mk
include $(TOPDIR)/include.mk
include $(TOPDIR)/lib.mk

$(GENSOURCES): $(TOPDIR)/pylib/templates/filter.point.c.t
