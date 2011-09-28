TOPDIR=../..

CSOURCES=$(shell echo *.c)

INCLUDE=core gfx SDL backends
LDLIBS+=-lGP -L$(TOPDIR)/build/ -lpng

APPS=grinder

grinder: params.o

include $(TOPDIR)/include.mk
include $(TOPDIR)/app.mk
