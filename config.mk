CFLAGS+=-W -Wall -Wextra -fPIC -O2 -std=gnu99
CFLAGS+=-I$(TOPDIR)/include/
LDLIBS+=-ldl

# path to local module directory
PYLIBSDIR=$(TOPDIR)/pylib

# Python binary/version
PYTHON_BIN=python
# To test with other python versions (example):
#PYTHON_BIN=${TOPDIR}/virtualpy2.4/bin/python

# Command to run Python with pylib/ modules
PYTHON=PYTHONPATH=$$PYTHONPATH:${PYLIBSDIR} ${PYTHON_BIN} -Werror
