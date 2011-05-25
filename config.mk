CFLAGS+=-W -Wall -Wextra -fPIC -ggdb -O2 -std=gnu99
CFLAGS+=-I$(TOPDIR)/include/
LDLIBS+=-ldl

# path to local module directory
PYLIBSDIR=$(TOPDIR)/pylib

# Command to run Python with pylib/ modules
PYTHON=PYTHONPATH=$$PYTHONPATH:${PYLIBSDIR} python
