TOPDIR=../..

include $(TOPDIR)/pre.mk

CSOURCES=Context.c Pixel.c BlitClipped.c Debug.c

GENSOURCES+=WritePixel.gen.c GetPutPixel.gen.c Convert.gen.c BlitConv.gen.c \
            Convert_Scale.gen.c GetSetBits.gen.c

APPS=WritePixel.gen Pixel Context GetPutPixel.gen Convert.gen BlitConv.gen \
     Convert_Scale.gen GetSetBits.gen BlitClipped Debug

include ../tests.mk

include $(TOPDIR)/gen.mk
include $(TOPDIR)/app.mk
include $(TOPDIR)/post.mk
