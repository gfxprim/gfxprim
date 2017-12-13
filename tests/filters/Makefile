TOPDIR=../..
include $(TOPDIR)/pre.mk

CSOURCES=FilterMirrorH.c common.c LinearConvolution.c

GENSOURCES=APICoverage.gen.c FiltersCompare.gen.c

APPS=FilterMirrorH APICoverage.gen FiltersCompare.gen LinearConvolution

include ../tests.mk

FilterMirrorH: common.o

include $(TOPDIR)/gen.mk
include $(TOPDIR)/app.mk
include $(TOPDIR)/post.mk
