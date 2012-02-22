TOPDIR=../..

LIBNAME=core
TESTSUITE=core_suite
LDLIBS+=-lGP -L$(TOPDIR)/build/ -lcheck -lm -lSDL
GENSOURCES+=GP_Convert.test.gen.c GP_WritePixel.test.gen.c \
            GP_MixPixels.test.gen.c

all: $(TESTSUITE)

include $(TOPDIR)/tests.mk
include $(TOPDIR)/include.mk

