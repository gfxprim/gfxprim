TOPDIR=../..

LIBNAME=core
TESTSUITE=core_suite
LDLIBS+=-lGP -L$(TOPDIR)/build/ -lcheck -lpng -ljpeg -lm -ldl
GENSOURCES+=GP_Convert.test.gen.c GP_WritePixel.test.gen.c

all: $(TESTSUITE)

include $(TOPDIR)/tests.mk
include $(TOPDIR)/include.mk

