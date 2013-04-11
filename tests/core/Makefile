TOPDIR=../..

include $(TOPDIR)/pre.mk

CSOURCES=Context.c Pixel.c

GENSOURCES+=WritePixel_testsuite.gen.c GetPutPixel.gen.c

APPS=WritePixel_testsuite.gen Pixel Context GetPutPixel.gen

include ../tests.mk

include $(TOPDIR)/gen.mk
include $(TOPDIR)/app.mk
include $(TOPDIR)/post.mk
