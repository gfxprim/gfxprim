TOPDIR=../..
include $(TOPDIR)/pre.mk

CSOURCES=$(shell echo *.c)

INCLUDE=
LDFLAGS+=-L$(TOPDIR)/build/
LDLIBS+=-lrt

LDLIBS+=$(LDLIBS_LOADERS) $(LDLIBS_BACKENDS)

APPS=spiv

spiv: cpu_timer.o image_cache.o image_list.o image_actions.o spiv_help.o\
      image_loader.o

include $(TOPDIR)/app.mk
include $(TOPDIR)/post.mk
