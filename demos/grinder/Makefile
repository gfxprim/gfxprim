TOPDIR=../..

CSOURCES=$(shell echo *.c)

INCLUDE=core gfx SDL
LDLIBS+=-lGP -L$(TOPDIR)/build/

APPS=grinder

grinder: params.o histogram.o

include $(TOPDIR)/pre.mk
include $(TOPDIR)/app.mk
include $(TOPDIR)/post.mk
