TOPDIR=../../..
LIBNAME=backends
INCLUDE=core

include $(TOPDIR)/pre.mk

LDLIBS+=$(LDLIBS_BACKENDS)

INSTALL_FILES+=_extend_backend.py

include $(TOPDIR)/pywrap.mk
include $(TOPDIR)/post.mk

include ../silence_swig_warnings.mk
