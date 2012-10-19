TOPDIR=../..

include $(TOPDIR)/pre.mk

CSOURCES=Context.c

# hack
LIBNAME=core

GENSOURCES+=WritePixel_testsuite.gen.c

APPS=WritePixel_testsuite.gen Context

include ../tests.mk

include $(TOPDIR)/gen.mk
include $(TOPDIR)/app.mk
include $(TOPDIR)/post.mk
