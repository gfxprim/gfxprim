TOPDIR=../..

CSOURCES=$(shell echo *.c)

INCLUDE=
CFLAGS+=-pthread
LDFLAGS+=-pthread
LDLIBS+=-lGP -lGP_backends -lSDL -L$(TOPDIR)/build/

APPS=fbshow

fbshow: cpu_timer.o

include $(TOPDIR)/include.mk
include $(TOPDIR)/app.mk
