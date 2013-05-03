include $(TOPDIR)/config.gen.mk

CFLAGS+=-I$(TOPDIR)/include/
LDFLAGS+=-L$(TOPDIR)/build/

# path to local module directory
PYLIBSDIR=$(TOPDIR)/pylib

ifdef PYTHON_CONFIG
PYTHON_INCLUDE=$(shell $(PYTHON_CONFIG) --include)
endif

# To test with other python versions (example):
#PYTHON_BIN=${TOPDIR}/virtualpy2.4/bin/python

# Command to run Python with pylib/ modules
PYTHON=PYTHONPATH=$$PYTHONPATH:${PYLIBSDIR} ${PYTHON_BIN} -Werror

SWIGOPTS=-w322,314 -I/usr/include/

# Linker flags
LDLIBS_GP=$(shell $(TOPDIR)/gfxprim-config --libs)
LDLIBS_LOADERS=$(shell $(TOPDIR)/gfxprim-config --libs-loaders)
LDLIBS_BACKENDS=$(shell $(TOPDIR)/gfxprim-config --libs-backends)
LDLIBS_GRABBERS=$(shell $(TOPDIR)/gfxprim-config --libs-grabbers)
