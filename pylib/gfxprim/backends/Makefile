TOPDIR=../../..
LIBNAME=backends
INCLUDE=core

LDFLAGS+=$(shell $(TOPDIR)/gfxprim-config --libs-backends)

include $(TOPDIR)/pre.mk
include $(TOPDIR)/pywrap.mk
include $(TOPDIR)/post.mk

include ../silence_swig_warnings.mk
