TOPDIR=../..

include $(TOPDIR)/pre.mk

CSOURCES=tbox.c tattr.c button.c checkbox.c tabs.c label.c

APPS=tbox tattr button checkbox tabs label

LDLIBS+=$(shell $(TOPDIR)/gfxprim-config --libs-widgets)

include ../tests.mk

include $(TOPDIR)/gen.mk
include $(TOPDIR)/app.mk
include $(TOPDIR)/post.mk
