TOPDIR=../..
include $(TOPDIR)/pre.mk

SOURCES=$(shell echo *.c)

ifneq ($(HAVE_LIBSDL),yes)
CSOURCES=$(filter-out SDL_glue.c,$(SOURCES))
else
CSOURCES=$(SOURCES)
endif

INCLUDE=
LDLIBS+=-lrt -lgfxprim

APPS=backend_example loaders_example loaders filters_symmetry gfx_koch\
     virtual_backend_example meta_data showimage\
     v4l2_show v4l2_grab convolution weighted_median shapetest koch \
     input_example fileview linetest randomshapetest fonttest\
     loaders_register blittest textaligntest x11_windows\
     debug_handler gaussian_noise version pretty_print timers\
     zip_container backend_timers_example memory_io data_storage\
     vector matrix proxy_backend bbox

ifeq ($(HAVE_LIBSDL),yes)
APPS+=SDL_glue

SDL_glue: LDLIBS+=$(shell sdl-config --libs) -lgfxprim-backends
endif

proxy_backend: LDLIBS+=-lgfxprim-backends
showimage: LDLIBS+=-lgfxprim-backends -lgfxprim-loaders
backend_example: LDLIBS+=-lgfxprim-backends
backend_timers_example: LDLIBS+=-lgfxprim-backends
virtual_backend_example: LDLIBS+=-lgfxprim-backends -lm
loaders_example: LDLIBS+=-lgfxprim-loaders
loaders: LDLIBS+=-lgfxprim-loaders
filters_symmetry: LDLIBS+=-lgfxprim-loaders
gfx_koch: LDLIBS+=-lgfxprim-loaders -lm
meta_data: LDLIBS+=-lgfxprim-loaders
v4l2_show: LDLIBS+=-lgfxprim-grabbers -lgfxprim-backends -lgfxprim-loaders
v4l2_grab: LDLIBS+=-lgfxprim-grabbers -lgfxprim-loaders
convolution: LDLIBS+=-lgfxprim-loaders
weighted_median: LDLIBS+=-lgfxprim-loaders
shapetest: LDLIBS+=-lgfxprim-backends
koch: LDLIBS+=-lgfxprim-backends -lm
input_example: LDLIBS+=-lgfxprim-backends
fileview: LDLIBS+=-lgfxprim-backends
linetest: LDLIBS+=-lgfxprim-backends -lm
randomshapetest: LDLIBS+=-lgfxprim-backends
fonttest: LDLIBS+=-lgfxprim-backends
textaligntest: LDLIBS+=-lgfxprim-backends
loaders_register: LDLIBS+=-lgfxprim-loaders
gaussian_noise: LDLIBS+=-lgfxprim-loaders
blittest: LDLIBS+=-lgfxprim-backends -lgfxprim-loaders
x11_windows: LDLIBS+=-lgfxprim-backends
zip_container: LDLIBS+=-lgfxprim-loaders -lgfxprim-backends
memory_io: LDLIBS+=-lgfxprim-backends -lgfxprim-loaders
data_storage: LDLIBS+=-lgfxprim-loaders

include $(TOPDIR)/app.mk
include $(TOPDIR)/post.mk
