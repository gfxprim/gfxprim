TOPDIR=../..
include $(TOPDIR)/pre.mk

CSOURCES=FilterMirrorH.c

GENSOURCES=APICoverage.gen.c

APPS=FilterMirrorH APICoverage.gen

include ../tests.mk

FilterMirrorH: common.o

CLEAN+=common.o

include $(TOPDIR)/gen.mk
include $(TOPDIR)/app.mk
include $(TOPDIR)/post.mk
