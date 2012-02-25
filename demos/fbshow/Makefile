TOPDIR=../..

CSOURCES=$(shell echo *.c)

INCLUDE=
CFLAGS+=-pthread
LDFLAGS+=-pthread -lrt
LDLIBS+=-lGP -lGP_backends -lSDL -L$(TOPDIR)/build/

APPS=fbshow

fbshow: cpu_timer.o

include $(TOPDIR)/pre.mk
include $(TOPDIR)/app.mk
include $(TOPDIR)/post.mk
