TOPDIR=../..

CSOURCES=$(shell echo *.c)

INCLUDE=loaders
LDLIBS+=-lGP -L$(TOPDIR)/build/

APPS=PBM_invert PBM_test PGM_invert PGM_test

include $(TOPDIR)/include.mk
include $(TOPDIR)/app.mk
