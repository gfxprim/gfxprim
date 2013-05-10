TOPDIR=../..
include $(TOPDIR)/pre.mk

CSOURCES=$(filter-out $(wildcard *.gen.c),$(wildcard *.c))
GENSOURCES=GP_Line.gen.c GP_HLine.gen.c GP_LineAA.gen.c GP_PutPixelAA.gen.c \
	GP_HLineAA.gen.c GP_VLineAA.gen.c GP_FillCircle.gen.c GP_VLine.gen.c \
	GP_FillEllipse.gen.c
LIBNAME=gfx

include $(TOPDIR)/gen.mk
include $(TOPDIR)/lib.mk
include $(TOPDIR)/post.mk
