TOPDIR=../..
include $(TOPDIR)/pre.mk

CSOURCES=$(shell echo *.c)

INCLUDE=
LDFLAGS+=-L$(TOPDIR)/build/

LDLIBS+=-lgfxprim-backends -lgfxprim -lvterm -lutil

APPS=termini

INSTALL_BIN=termini

include $(TOPDIR)/app.mk
include $(TOPDIR)/install.mk
include $(TOPDIR)/post.mk
