TOPDIR=../..

CSOURCES=$(shell echo *.c)

INCLUDE=
LDFLAGS+=-L$(TOPDIR)/build/
LDLIBS+=-lrt `$(TOPDIR)/gfxprim-config --libs --libs-backends`

APPS=particle_demo

$(APPS): space.o

include $(TOPDIR)/pre.mk
include $(TOPDIR)/app.mk
include $(TOPDIR)/post.mk
