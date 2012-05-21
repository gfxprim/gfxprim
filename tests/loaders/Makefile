TOPDIR=../..
include $(TOPDIR)/pre.mk

CSOURCES=$(shell echo *.c)

INCLUDE=loaders
LDLIBS+=-lGP -L$(TOPDIR)/build/

APPS=PBM_invert PBM_test PGM_invert PGM_test

include $(TOPDIR)/app.mk
include $(TOPDIR)/post.mk
