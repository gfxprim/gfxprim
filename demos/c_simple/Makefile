TOPDIR=../..

CSOURCES=$(shell echo *.c)

INCLUDE=
LDLIBS+=-lGP -lGP_backends -lSDL -L$(TOPDIR)/build/

APPS=backend_example loaders_example loaders filters_symmetry

include $(TOPDIR)/pre.mk
include $(TOPDIR)/app.mk
include $(TOPDIR)/post.mk
