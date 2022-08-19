TOPDIR=../..
include $(TOPDIR)/pre.mk

CSOURCES=$(shell echo *.c)

INCLUDE=
LDFLAGS+=-L$(TOPDIR)/build/
LDLIBS+=-lrt
CFLAGS+=-pthread

LDLIBS+=-lgfxprim-loaders -lgfxprim-backends -lgfxprim

APPS=spiv

INSTALL_BIN=spiv
INSTALL_MAN=spiv.1

spiv: cpu_timer.o image_cache.o image_list.o image_actions.o spiv_help.o\
      image_loader.o cfg.o spiv_config.o

include $(TOPDIR)/app.mk
include $(TOPDIR)/install.mk
include $(TOPDIR)/post.mk
