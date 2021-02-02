TOPDIR=../..

include $(TOPDIR)/pre.mk

CSOURCES=tbox.c tattr.c button.c checkbox.c tabs.c

APPS=tbox tattr button checkbox tabs

LDLIBS+=$(shell $(TOPDIR)/gfxprim-config --libs-widgets)

include ../tests.mk

include $(TOPDIR)/gen.mk
include $(TOPDIR)/app.mk
include $(TOPDIR)/post.mk
