TOPDIR=../..

include $(TOPDIR)/pre.mk

#CSOURCES=$(shell echo *.c)

# hack
LIBNAME=core

LDFLAGS+=-L../framework/ -L$(TOPDIR)/build/
LDLIBS+=$(shell $(TOPDIR)/gfxprim-config --libs --libs-loaders)
LDLIBS+=-ltst_preload -ldl -ltst
CFLAGS+=-I../framework/

GENSOURCES+=WritePixel_testsuite.gen.c

APPS=WritePixel_testsuite.gen

$(APPS): ../framework/libtst.a

CLEAN+=log.html log.json

include $(TOPDIR)/gen.mk
include $(TOPDIR)/app.mk
include $(TOPDIR)/post.mk
