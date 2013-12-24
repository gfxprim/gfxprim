TOPDIR=../..

CSOURCES=$(shell echo *.c)

LDLIBS+=-lgfxprim -lgfxprim-backends -L$(TOPDIR)/build/

APPS=$(CSOURCES:.c=)

include $(TOPDIR)/pre.mk
include $(TOPDIR)/app.mk
include $(TOPDIR)/post.mk
