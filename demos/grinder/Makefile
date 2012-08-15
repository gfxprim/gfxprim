TOPDIR=../..

CSOURCES=$(shell echo *.c)

INCLUDE=core gfx
LDFLAGS+=-L$(TOPDIR)/build/
LDLIBS+=$(shell $(TOPDIR)/gfxprim-config --libs --libs-backends) -lrt

APPS=grinder

grinder: params.o histogram.o

include $(TOPDIR)/pre.mk
include $(TOPDIR)/app.mk
include $(TOPDIR)/post.mk
