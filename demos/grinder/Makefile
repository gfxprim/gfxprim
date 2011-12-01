TOPDIR=../..

CSOURCES=$(shell echo *.c)

INCLUDE=core gfx SDL backends
LDLIBS+=-lGP -L$(TOPDIR)/build/

APPS=grinder

grinder: params.o histogram.o

include $(TOPDIR)/include.mk
include $(TOPDIR)/app.mk
