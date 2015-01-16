TOPDIR=../..
include $(TOPDIR)/pre.mk

CSOURCES=$(shell echo *.c)

INCLUDE=
LDLIBS+=-lgfxprim-backends -lgfxprim-loaders

APPS=bogoman

bogoman: bogoman_map.o bogoman_debug.o bogoman_loader.o bogoman_render.o

include $(TOPDIR)/app.mk
include $(TOPDIR)/post.mk
