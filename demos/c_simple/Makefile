TOPDIR=../..

CSOURCES=$(shell echo *.c)

INCLUDE=
LDLIBS+=-lGP -lGP_backends -lSDL -L$(TOPDIR)/build/

APPS=backend_example loaders_example

include $(TOPDIR)/pre.mk
include $(TOPDIR)/app.mk
include $(TOPDIR)/post.mk
