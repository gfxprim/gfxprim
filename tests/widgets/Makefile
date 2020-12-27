TOPDIR=../..

include $(TOPDIR)/pre.mk

CSOURCES=tbox.c

APPS=tbox

LDLIBS+=$(shell $(TOPDIR)/gfxprim-config --libs-widgets)

include ../tests.mk

include $(TOPDIR)/gen.mk
include $(TOPDIR)/app.mk
include $(TOPDIR)/post.mk
