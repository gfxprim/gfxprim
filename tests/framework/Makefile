TOPDIR=../..
include $(TOPDIR)/pre.mk

CSOURCES=$(shell echo *.c)

INCLUDE=
LDFLAGS+=-L.
LDLIBS+=-ltst_preload -ldl -ltst
CFLAGS+=

test: libtst.a

APPS=test

ALL+=libtst_preload.so libtst.a

libtst_preload.so: tst_preload.o tst_alloc_barriers.o tst_preload_FILE.o
	gcc -Wl,-soname -Wl,tst_preload.so --shared -ldl -fPIC $^ -o $@

libtst.a: tst_test.o tst_job.o tst_msg.o tst_log.o tst_main.o
	ar rcs $@ $^

CLEAN+=libtst_preload.so libtst.a log.html log.json

include $(TOPDIR)/app.mk
include $(TOPDIR)/post.mk
