TOPDIR=../..
include $(TOPDIR)/pre.mk

CSOURCES=$(shell echo *.c)

INCLUDE=
LDFLAGS+=-L.
LDLIBS+=-ltst_preload -ldl
CFLAGS+=

test: tst_test.o tst_job.o tst_msg.o

APPS=test

ALL+=libtst_preload.so

libtst_preload.so: tst_preload.o
	gcc -Wl,-soname -Wl,tst_preload.so --shared -ldl -fPIC $< -o $@

CLEAN+=libtst_preload.so

include $(TOPDIR)/app.mk
include $(TOPDIR)/post.mk
