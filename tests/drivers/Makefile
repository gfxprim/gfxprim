TOPDIR=../..

CSOURCES=$(shell echo *.c)

LDLIBS+=-lGP -lGP_backends -lSDL -L$(TOPDIR)/build/

APPS=$(CSOURCES:.c=)

include $(TOPDIR)/include.mk
include $(TOPDIR)/app.mk
