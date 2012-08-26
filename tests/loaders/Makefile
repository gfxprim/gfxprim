TOPDIR=../..
include $(TOPDIR)/pre.mk

CSOURCES=$(shell echo *.c)

LDFLAGS+=-L../framework/
LDLIBS+=$(shell $(TOPDIR)/gfxprim-config --libs --libs-loaders)
LDLIBS+=-ltst_preload -ldl -ltst
CFLAGS+=-I../framework/

APPS=loaders_suite

CLEAN+=libtst_preload.so log.html log.json

include $(TOPDIR)/app.mk
include $(TOPDIR)/post.mk
