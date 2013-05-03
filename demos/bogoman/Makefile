TOPDIR=../..

CSOURCES=$(shell echo *.c)

INCLUDE=
LDLIBS+=$(LDLIBS_BACKENDS)

APPS=bogoman

bogoman: bogoman_map.o bogoman_debug.o bogoman_loader.o bogoman_render.o

include $(TOPDIR)/pre.mk
include $(TOPDIR)/app.mk
include $(TOPDIR)/post.mk
