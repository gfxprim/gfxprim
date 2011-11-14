TOPDIR=../..

CSOURCES=$(shell echo *.c)

LDLIBS+=-lGP -lGP_SDL -lSDL -L$(TOPDIR)/build/

APPS=$(CSOURCES:.c=)

include $(TOPDIR)/include.mk
include $(TOPDIR)/app.mk
