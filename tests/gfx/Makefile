TOPDIR=../..
include $(TOPDIR)/pre.mk

CSOURCES=$(shell echo *.c)

APPS=gfx_benchmark Circle FillCircle Line CircleSeg Polygon

Circle: common.o
FillCircle: common.o
Line: common.o
CircleSeg: common.o
Polygon: common.o

include ../tests.mk

include $(TOPDIR)/app.mk
include $(TOPDIR)/post.mk
