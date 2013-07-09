TOPDIR=../..

include $(TOPDIR)/pre.mk

SOURCES=$(shell echo *.c)

ifneq ($(HAVE_LIBSDL),yes)
CSOURCES=$(filter-out SDL_glue.c,$(SOURCES))
else
CSOURCES=$(SOURCES)
endif

INCLUDE=
LDLIBS+=-lrt

APPS=backend_example loaders_example loaders filters_symmetry gfx_koch\
     virtual_backend_example meta_data meta_data_dump tmp_file showimage\
     v4l2_show v4l2_grab convolution weighted_median shapetest koch \
     input_example fileview linetest randomshapetest fonttest\
     loaders_register blittest textaligntest abort sin_AA x11_windows\
     debug_handler gaussian_noise byte_utils version pretty_print timers\
     zip_container

ifeq ($(HAVE_LIBSDL),yes)
APPS+=SDL_glue

SDL_glue: LDLIBS+=$(LDLIBS_BACKENDS)
endif

showimage: LDLIBS+=$(LDLIBS_BACKENDS) $(LDLIBS_LOADERS)
backend_example: LDLIBS+=$(LDLIBS_BACKENDS)
virtual_backend_example: LDLIBS+=$(LDLIBS_BACKENDS)
loaders_example: LDLIBS+=$(LDLIBS_LOADERS)
loaders: LDLIBS+=$(LDLIBS_LOADERS)
tmp_file: LDLIBS+=$(LDLIBS_LOADERS)
filters_symmetry: LDLIBS+=$(LDLIBS_LOADERS)
gfx_koch: LDLIBS+=$(LDLIBS_LOADERS)
meta_data: LDLIBS+=$(LDLIBS_LOADERS)
meta_data_dump: LDLIBS+=$(LDLIBS_LOADERS)
v4l2_show: LDLIBS+=$(LDLIBS_GRABBERS) $(LDLIBS_BACKENDS)
v4l2_grab: LDLIBS+=$(LDLIBS_GRABBERS) $(LDLIBS_LOADERS)
convolution: LDLIBS+=$(LDLIBS_LOADERS)
weighted_median: LDLIBS+=$(LDLIBS_LOADERS)
shapetest: LDLIBS+=$(LDLIBS_BACKENDS)
koch: LDLIBS+=$(LDLIBS_BACKENDS)
input_example: LDLIBS+=$(LDLIBS_BACKENDS)
fileview: LDLIBS+=$(LDLIBS_BACKENDS)
linetest: LDLIBS+=$(LDLIBS_BACKENDS)
randomshapetest: LDLIBS+=$(LDLIBS_BACKENDS)
fonttest: LDLIBS+=$(LDLIBS_BACKENDS)
textaligntest: LDLIBS+=$(LDLIBS_BACKENDS)
loaders_register: LDLIBS+=$(LDLIBS_LOADERS)
gaussian_noise: LDLIBS+=$(LDLIBS_LOADERS)
byte_utils: LDLIBS+=$(LDLIBS_LOADERS)
blittest: LDLIBS+=$(LDLIBS_BACKENDS) $(LDLIBS_LOADERS)
sin_AA: LDLIBS+=$(LDLIBS_BACKENDS)
x11_windows: LDLIBS+=$(LDLIBS_BACKENDS)
zip_container: LDLIBS+=$(LDLIBS_LOADERS) $(LDLIBS_BACKENDS)

include $(TOPDIR)/app.mk
include $(TOPDIR)/post.mk
