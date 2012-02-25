TOPDIR=..
SUBDIRS=gfxprim

#
# Add .pyc files to CLEAN list
#
PYTHON_FILES=$(shell find "${PYLIBSDIR}" -name '*.py')
CLEAN+=$(patsubst %.py, %.pyc, ${PYTHON_FILES})

include $(TOPDIR)/post.mk
