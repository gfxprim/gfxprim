TOPDIR=../..
include $(TOPDIR)/pre.mk

CSOURCES=$(filter-out $(wildcard *.gen.c),$(wildcard *.c))
GENSOURCES=api_coverage.gen.c line_symmetry.gen.c fill_triangle.gen.c

APPS=gfx_benchmark circle fill_circle line circle_seg polygon ellipse hline\
     vline fill_ellipse fill_rect api_coverage.gen\
     line_symmetry.gen fill_triangle.gen fill_triangle

circle: common.o
fill_circle: common.o
ellipse: common.o
fill_ellipse: common.o
line: common.o
circle_seg: common.o
polygon: common.o
hline: common.o
vline: common.o
fill_rect: common.o
fill_triangle: common.o

include ../tests.mk

include $(TOPDIR)/gen.mk
include $(TOPDIR)/app.mk
include $(TOPDIR)/post.mk
