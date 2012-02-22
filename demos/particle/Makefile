TOPDIR=../..

CSOURCES=$(shell echo *.c)

INCLUDE=
LDLIBS+=-lGP -lGP_backends -lSDL -L$(TOPDIR)/build/

APPS=particle_demo

$(APPS): space.o

include $(TOPDIR)/include.mk
include $(TOPDIR)/app.mk
