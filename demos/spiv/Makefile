TOPDIR=../..

CSOURCES=$(shell echo *.c)

INCLUDE=
LDFLAGS+=-L$(TOPDIR)/build/
LDLIBS+=-lrt

LDLIBS+=$(LDLIBS_LOADERS) $(LDLIBS_BACKENDS)

APPS=spiv

spiv: cpu_timer.o image_cache.o image_list.o

include $(TOPDIR)/pre.mk
include $(TOPDIR)/app.mk
include $(TOPDIR)/post.mk
