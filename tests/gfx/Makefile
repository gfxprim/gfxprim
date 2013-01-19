TOPDIR=../..
include $(TOPDIR)/pre.mk

CSOURCES=$(shell echo *.c)

APPS=gfx_benchmark Circle FillCircle Line CircleSeg Polygon Ellipse HLine\
     VLine

Circle: common.o
FillCircle: common.o
Ellipse: common.o
Line: common.o
CircleSeg: common.o
Polygon: common.o
HLine: common.o
VLine: common.o

include ../tests.mk

include $(TOPDIR)/app.mk
include $(TOPDIR)/post.mk
