TOPDIR=../..
include $(TOPDIR)/pre.mk

SOURCES=$(shell echo *.c)

SDL2_CONFIG=$(shell which sdl2-config 2>/dev/null)
SDL_CONFIG=$(shell which sdl-config 2>/dev/null)

ifeq ($(SDL_CONFIG),)
FILTER_OUT+=SDL_glue.c
$(info Disabling SDL_glue)
endif

ifeq ($(SDL2_CONFIG),)
FILTER_OUT+=SDL2_glue.c
$(info Disabling SDL2_glue)
endif

CSOURCES=$(filter-out $(FILTER_OUT), $(SOURCES))

INCLUDE=
LDLIBS+=-lrt -lgfxprim

APPS=backend_example loaders_example loaders filters_symmetry gfx_koch\
     meta_data showimage v4l2_show v4l2_grab convolution weighted_median\
     shapetest koch input_example fileview linetest randomshapetest\
     fonttest loaders_register blittest textaligntest x11_windows\
     debug_handler gaussian_noise version pretty_print timers\
     zip_container backend_timers_example memory_io data_storage\
     vector matrix proxy_backend bbox backend_tasks_example json_writer\
     draw_glyph dpi_check unicodetest backend_cursor json_writer_file\
     elf_note update_rect_test

ifneq ($(SDL_CONFIG),)
APPS+=SDL_glue
SDL_glue: LDLIBS+=$(shell sdl-config --libs)
endif

ifneq ($(SDL2_CONFIG),)
APPS+=SDL2_glue
SDL2_glue: LDLIBS+=$(shell sdl2-config --libs)
endif

update_rect_test: LDLIBS+=-lgfxprim-backends
backend_tasks_example: LDLIBS+=-lgfxprim-backends
proxy_backend: LDLIBS+=-lgfxprim-backends
showimage: LDLIBS+=-lgfxprim-backends -lgfxprim-loaders
backend_example: LDLIBS+=-lgfxprim-backends
backend_cursor: LDLIBS+=-lgfxprim-backends
backend_sw_cursor: LDLIBS+=-lgfxprim-backends
backend_timers_example: LDLIBS+=-lgfxprim-backends
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
unicodetest: LDLIBS+=-lgfxprim-backends
textaligntest: LDLIBS+=-lgfxprim-backends
loaders_register: LDLIBS+=-lgfxprim-loaders
gaussian_noise: LDLIBS+=-lgfxprim-loaders
blittest: LDLIBS+=-lgfxprim-backends -lgfxprim-loaders
x11_windows: LDLIBS+=-lgfxprim-backends
zip_container: LDLIBS+=-lgfxprim-loaders -lgfxprim-backends
memory_io: LDLIBS+=-lgfxprim-backends -lgfxprim-loaders
data_storage: LDLIBS+=-lgfxprim-loaders
draw_glyph: LDLIBS+=-lgfxprim-backends
dpi_check: LDLIBS+=-lgfxprim-backends

include $(TOPDIR)/app.mk
include $(TOPDIR)/post.mk
