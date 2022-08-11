TOPDIR=../..
include $(TOPDIR)/pre.mk

APPS=loaders

CSOURCES=$(shell echo *.c)

LDLIBS=$(shell $(TOPDIR)/gfxprim-config --libs-loaders --libs)
LDFLAGS=$(shell $(TOPDIR)/gfxprim-config --ldflags)

afl: loaders
	./run_afl.sh

include $(TOPDIR)/app.mk
include $(TOPDIR)/post.mk
