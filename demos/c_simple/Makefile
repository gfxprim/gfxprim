TOPDIR=../..

include $(TOPDIR)/pre.mk

SOURCES=$(shell echo *.c)

ifneq ($(HAVE_LIBSDL),yes)
CSOURCES=$(filter-out SDL_glue.c,$(SOURCES))
else
CSOURCES=$(SOURCES)
endif

INCLUDE=
LDFLAGS+=-L$(TOPDIR)/build/
LDLIBS+=-lrt `$(TOPDIR)/gfxprim-config --libs --libs-backends`

APPS=backend_example loaders_example loaders filters_symmetry gfx_koch\
     virtual_backend_example meta_data meta_data_dump tmp_file showimage\
     v4l2_show v4l2_grab convolution weighted_median shapetest koch \
     input_example fileview linetest randomshapetest fonttest\
     loaders_register blittest textaligntest

ifeq ($(HAVE_LIBSDL),yes)
APPS+=SDL_glue
endif

v4l2_show: LDLIBS+=-lGP_grabbers
v4l2_grab: LDLIBS+=-lGP_grabbers

include $(TOPDIR)/app.mk
include $(TOPDIR)/post.mk
