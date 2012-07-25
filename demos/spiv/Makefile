TOPDIR=../..

CSOURCES=$(shell echo *.c)

INCLUDE=
LDFLAGS+=-L$(TOPDIR)/build/
LDLIBS+=-lrt `$(TOPDIR)/gfxprim-config --libs --libs-backends`

APPS=spiv

spiv: cpu_timer.o image_cache.o

include $(TOPDIR)/pre.mk
include $(TOPDIR)/app.mk
include $(TOPDIR)/post.mk
