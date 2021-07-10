TOPDIR=../..
include $(TOPDIR)/pre.mk

CSOURCES=$(filter-out $(wildcard *.gen.c),$(wildcard *.c))
GENSOURCES=gp_line.gen.c gp_hline.gen.c gp_fill_circle.gen.c gp_vline.gen.c \
           gp_fill_ellipse.gen.c gp_fill_triangle.gen.c gp_circle.gen.c \
	   gp_circle_seg.gen.c gp_symbol.gen.c gp_fill_ring.gen.c gp_polygon.gen.c \
	   gp_line_th.gen.c

LIBNAME=gfx

include $(TOPDIR)/gen.mk
include $(TOPDIR)/lib.mk
include $(TOPDIR)/post.mk
