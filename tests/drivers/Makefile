TOPDIR=../..

CSOURCES=$(shell echo *.c)

LDLIBS+=-lGP -lGP_backends -L$(TOPDIR)/build/

APPS=$(CSOURCES:.c=)

include $(TOPDIR)/pre.mk
include $(TOPDIR)/app.mk
include $(TOPDIR)/post.mk
