TOPDIR=../..

CSOURCES=$(shell echo *.c)

INCLUDE=filters loaders
LDLIBS+=-lGP -L$(TOPDIR)/build/

APPS=PGM_rotate rotate_test

include $(TOPDIR)/include.mk
include $(TOPDIR)/app.mk
