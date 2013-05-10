TOPDIR=../..

include $(TOPDIR)/pre.mk

CSOURCES=Context.c Pixel.c

GENSOURCES+=WritePixel.gen.c GetPutPixel.gen.c BlitConv.gen.c

APPS=WritePixel.gen Pixel Context GetPutPixel.gen BlitConv.gen

include ../tests.mk

include $(TOPDIR)/gen.mk
include $(TOPDIR)/app.mk
include $(TOPDIR)/post.mk
