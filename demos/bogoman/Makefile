TOPDIR=../..

CSOURCES=$(shell echo *.c)

INCLUDE=
LDFLAGS+=-L$(TOPDIR)/build/
LDLIBS+=`$(TOPDIR)/gfxprim-config --libs --libs-backends`

APPS=bogoman

bogoman: bogoman_map.o bogoman_debug.o bogoman_loader.o bogoman_render.o

include $(TOPDIR)/pre.mk
include $(TOPDIR)/app.mk
include $(TOPDIR)/post.mk
