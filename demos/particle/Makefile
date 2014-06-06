TOPDIR=../..

CSOURCES=$(shell echo *.c)

INCLUDE=
LDFLAGS+=-L$(TOPDIR)/build/
LDLIBS+=-lrt -lm -lgfxprim-backends

APPS=particle_demo

$(APPS): space.o

include $(TOPDIR)/pre.mk
include $(TOPDIR)/app.mk
include $(TOPDIR)/post.mk
