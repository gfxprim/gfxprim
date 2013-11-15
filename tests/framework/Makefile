TOPDIR=../..
include $(TOPDIR)/pre.mk

CSOURCES=$(shell echo *.c)

INCLUDE=
LDFLAGS+=-L.
LDLIBS=-ltst_preload -lm -ldl -ltst -lrt

test: libtst.a

APPS=test

ALL+=libtst_preload.so libtst.a

libtst_preload.so: tst_preload.o tst_malloc_canaries.o tst_preload_FILE.o
ifndef VERBOSE
	@echo "LD   libtst_preload.so"
	@$(CC) -Wl,-soname -Wl,tst_preload.so --shared -fPIC $^ -ldl -o $@
else
	$(CC) -Wl,-soname -Wl,tst_preload.so --shared -fPIC $^ -ldl -o $@
endif

libtst.a: tst_suite.o tst_job.o tst_msg.o tst_log.o tst_main.o tst_timespec.o
ifndef VERBOSE
	@echo "AR   libtst.a"
	@$(AR) rcs $@ $^
else
	$(AR) rcs $@ $^
endif

CLEAN+=libtst_preload.so libtst.a log.json

include $(TOPDIR)/app.mk
include $(TOPDIR)/post.mk
