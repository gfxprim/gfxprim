TOPDIR=../..

include $(TOPDIR)/pre.mk

CSOURCES=tbox.c tattr.c button.c checkbox.c tabs.c label.c grid.c size_units.c

APPS=tbox tattr button checkbox tabs label grid size_units

LDLIBS+=$(shell $(TOPDIR)/gfxprim-config --libs-widgets)

include ../tests.mk

include $(TOPDIR)/gen.mk
include $(TOPDIR)/app.mk
include $(TOPDIR)/post.mk
