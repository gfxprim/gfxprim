TOPDIR=../..

CSOURCES=$(shell echo *.c)

INCLUDE=
LDFLAGS+=-L$(TOPDIR)/build/
LDLIBS+=-lrt `$(TOPDIR)/gfxprim-config --libs --libs-backends`

APPS=backend_example loaders_example loaders filters_symmetry gfx_koch\
     virtual_backend_example meta_data meta_data_dump tmp_file showimage\
     v4l2_show v4l2_grab convolution weighted_median shapetest koch input\
     fileview linetest

v4l2_show: LDLIBS+=-lGP_grabbers
v4l2_grab: LDLIBS+=-lGP_grabbers

include $(TOPDIR)/pre.mk
include $(TOPDIR)/app.mk
include $(TOPDIR)/post.mk
