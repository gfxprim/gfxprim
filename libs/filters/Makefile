TOPDIR=../..
include $(TOPDIR)/pre.mk

STATS_FILTERS=GP_Histogram.gen.c

POINT_FILTERS=GP_Invert.gen.c\
              GP_Brightness.gen.c GP_Contrast.gen.c\
	      GP_BrightnessContrast.gen.c GP_Posterize.gen.c\
              GP_GaussianNoise.gen.c GP_ApplyTables.gen.c \
              GP_MultiTone.gen.c

ARITHMETIC_FILTERS=GP_Diff.gen.c GP_Add.gen.c GP_Min.gen.c\
                   GP_Max.gen.c GP_Mul.gen.c

RESAMPLING_FILTERS=GP_ResizeNN.gen.c GP_Cubic.gen.c GP_ResizeCubic.gen.c\
                   GP_ResizeLinear.gen.c

GENSOURCES=GP_MirrorH.gen.c GP_Rotate.gen.c GP_FloydSteinberg.gen.c GP_HilbertPeano.gen.c\
           $(POINT_FILTERS) $(ARITHMETIC_FILTERS) $(STATS_FILTERS) $(RESAMPLING_FILTERS)\
	   GP_LinearConvolution.gen.c

CSOURCES=$(filter-out $(wildcard *.gen.c),$(wildcard *.c))
LIBNAME=filters
INCLUDE=core

include $(TOPDIR)/gen.mk
include $(TOPDIR)/lib.mk
include $(TOPDIR)/post.mk
