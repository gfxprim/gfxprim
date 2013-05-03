TOPDIR=../..
include $(TOPDIR)/pre.mk

CSOURCES=$(shell echo *.c)
INCLUDE=core gfx

LDLIBS+=-lrt -lm
LDLIBS+=$(LDLIBS_LOADERS)

APPS=grinder

grinder: params.o histogram.o

include $(TOPDIR)/app.mk
include $(TOPDIR)/post.mk
