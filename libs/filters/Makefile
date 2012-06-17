TOPDIR=../..

STATS_FILTERS=GP_Histogram.gen.c

POINT_FILTERS=GP_Contrast.gen.c GP_Brightness.gen.c GP_Invert.gen.c\
              GP_Point.gen.c GP_Noise.gen.c

ARITHMETIC_FILTERS=GP_Difference.gen.c GP_Addition.gen.c GP_Min.gen.c\
                   GP_Max.gen.c GP_Multiply.gen.c

RESAMPLING_FILTERS=GP_ResizeNN.gen.c GP_Cubic.gen.c

GENSOURCES=GP_MirrorV.gen.c GP_Rotate.gen.c GP_FloydSteinberg.gen.c GP_HilbertPeano.gen.c\
           $(POINT_FILTERS) $(ARITHMETIC_FILTERS) $(STATS_FILTERS) $(RESAMPLING_FILTERS)

CSOURCES=$(filter-out $(wildcard *.gen.c),$(wildcard *.c))
LIBNAME=filters
INCLUDE=core

include $(TOPDIR)/pre.mk
include $(TOPDIR)/gen.mk
include $(TOPDIR)/lib.mk
include $(TOPDIR)/post.mk

$(POINT_FILTERS) $(ARITHMETIC_FILTERS) $(STATS_FILTERS): $(TEMPLATE_DIR)/filter.c.t
$(STATS_FILTERS): $(TEMPLATE_DIR)/filter.stats.c.t
$(POINT_FILTERS): $(TEMPLATE_DIR)/filter.point.c.t
$(ARITHMETIC_FILTERS): $(TEMPLATE_DIR)/filter.arithmetic.c.t
