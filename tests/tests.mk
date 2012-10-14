# Constants for tests build

LDFLAGS+=-L../framework/ -L$(TOPDIR)/build/
LDLIBS+=$(shell $(TOPDIR)/gfxprim-config --libs --libs-loaders)
LDLIBS+=-ltst_preload -ldl -ltst -lrt
CFLAGS+=-I../framework/

$(APPS): ../framework/libtst.a

CLEAN+=log.html log.json
