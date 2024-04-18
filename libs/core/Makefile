TOPDIR=../..
include $(TOPDIR)/pre.mk

GENSOURCES=gp_pixel.gen.c gp_blit.gen.c gp_convert.gen.c \
           gp_srgb_correction.gen.c gp_fill.gen.c

ALL_SOURCES=$(filter-out $(wildcard *.gen.c),$(wildcard *.c))

ifneq ($(HAVE_PTHREAD),yes)
CSOURCES=$(filter-out gp_threads.c,$(ALL_SOURCES))
else
CSOURCES=$(ALL_SOURCES)
endif

LIBNAME=core

include $(TOPDIR)/gen.mk
include $(TOPDIR)/lib.mk
include $(TOPDIR)/post.mk
