TOPDIR=../..
include $(TOPDIR)/pre.mk

CSOURCES=$(shell echo *.c)

APPS=gfx_benchmark Circle

Circle: common.o

include ../tests.mk

include $(TOPDIR)/app.mk
include $(TOPDIR)/post.mk
