TOPDIR=../..
include $(TOPDIR)/pre.mk

CSOURCES=$(shell echo *.c)

APPS=gfx_benchmark Circle CircleSeg

Circle: common.o
CircleSeg: common.o

include ../tests.mk

include $(TOPDIR)/app.mk
include $(TOPDIR)/post.mk
