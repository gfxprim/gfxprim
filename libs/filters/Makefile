TOPDIR=../..
include $(TOPDIR)/pre.mk

STATS_FILTERS=gp_histogram.gen.c

POINT_FILTERS=gp_invert.gen.c\
              gp_brightness.gen.c gp_contrast.gen.c\
	      gp_brightness_contrast.gen.c gp_posterize.gen.c\
              gp_gaussian_noise.gen.c gp_apply_tables.gen.c \
              gp_multi_tone.gen.c

ARITHMETIC_FILTERS=gp_diff.gen.c gp_add.gen.c gp_min.gen.c\
                   gp_max.gen.c gp_mul.gen.c

RESAMPLING_FILTERS=gp_resize_nn.gen.c gp_cubic.gen.c gp_resize_cubic.gen.c\
                   gp_resize_linear.gen.c

GENSOURCES=gp_mirror_h.gen.c gp_rotate.gen.c gp_floyd_steinberg.gen.c gp_hilbert_peano.gen.c\
           $(POINT_FILTERS) $(ARITHMETIC_FILTERS) $(STATS_FILTERS) $(RESAMPLING_FILTERS)\
	   gp_linear_convolution.gen.c

CSOURCES=$(filter-out $(wildcard *.gen.c),$(wildcard *.c))
LIBNAME=filters
INCLUDE=core

include $(TOPDIR)/gen.mk
include $(TOPDIR)/lib.mk
include $(TOPDIR)/post.mk
