TOPDIR=../..

CSOURCES=$(shell echo *.c)

LDLIBS+=-lGP -lGP_backends -lSDL -L$(TOPDIR)/build/

APPS=$(CSOURCES:.c=)

include $(TOPDIR)/pre.mk
include $(TOPDIR)/app.mk
include $(TOPDIR)/post.mk
