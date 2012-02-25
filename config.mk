include $(TOPDIR)/config.gen.mk
CFLAGS+=-I$(TOPDIR)/include/

# path to local module directory
PYLIBSDIR=$(TOPDIR)/pylib

PYTHON_INCLUDE=`python-config --include`

# To test with other python versions (example):
#PYTHON_BIN=${TOPDIR}/virtualpy2.4/bin/python

# Command to run Python with pylib/ modules
PYTHON=PYTHONPATH=$$PYTHONPATH:${PYLIBSDIR} ${PYTHON_BIN} -Werror

SWIGOPTS=-Wextra -w322,314 -I/usr/include/

SWIG=swig

#
# If set to yes, rebuilds libGP* after typing make
# in any of the libs/*/ directories that are linked
# to libGP*.
#
REBUILD_LIBGP=yes
