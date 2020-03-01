TOPDIR=../..
include $(TOPDIR)/pre.mk

GENSOURCES=gp_pixel.gen.c gp_blit.gen.c gp_convert.gen.c \
           gp_gamma_correction.gen.c gp_fill.gen.c

CSOURCES=$(filter-out $(wildcard *.gen.c),$(wildcard *.c))
LIBNAME=core

gp_write_pixel.o: CFLAGS+=$(CFLAGS_WNIF)

include $(TOPDIR)/gen.mk
include $(TOPDIR)/lib.mk
include $(TOPDIR)/post.mk
