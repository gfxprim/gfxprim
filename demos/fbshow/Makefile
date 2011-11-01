TOPDIR=../..

CSOURCES=$(shell echo *.c)

INCLUDE=
CFLAGS+=-pthread
LDLIBS+=-lGP -L$(TOPDIR)/build/

APPS=fbshow

include $(TOPDIR)/include.mk
include $(TOPDIR)/app.mk
