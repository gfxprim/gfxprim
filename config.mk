include $(TOPDIR)/config.gen.mk
CFLAGS+=-I$(TOPDIR)/include/

# path to local module directory
PYLIBSDIR=$(TOPDIR)/pylib

PYTHON_INCLUDE=/usr/include/python2.6

# To test with other python versions (example):
#PYTHON_BIN=${TOPDIR}/virtualpy2.4/bin/python

# Command to run Python with pylib/ modules
PYTHON=PYTHONPATH=$$PYTHONPATH:${PYLIBSDIR} ${PYTHON_BIN} -Werror
