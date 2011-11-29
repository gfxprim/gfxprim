TOPDIR=../..

POINT_FILTERS=GP_Contrast.gen.c GP_Brightness.gen.c GP_Invert.gen.c\
              GP_Point.gen.c

ARITHMETIC_FILTERS=GP_Difference.gen.c GP_Addition.gen.c GP_Min.gen.c\
                   GP_Max.gen.c GP_Multiply.gen.c

GENSOURCES=GP_MirrorV.gen.c GP_Rotate.gen.c GP_Dither.gen.c\
           $(POINT_FILTERS) $(ARITHMETIC_FILTERS)

CSOURCES=$(filter-out $(wildcard *.gen.c),$(wildcard *.c))
LIBNAME=filters
INCLUDE=core

include $(TOPDIR)/gen.mk
include $(TOPDIR)/include.mk
include $(TOPDIR)/lib.mk

$(POINT_FILTERS) $(ARITHMETIC_FILTERS): $(TEMPLATE_DIR)/filter.c.t
$(POINT_FILTERS): $(TEMPLATE_DIR)/filter.point.c.t
$(ARITHMETIC_FILTERS): $(TEMPLATE_DIR)/filter.arithmetic.c.t
