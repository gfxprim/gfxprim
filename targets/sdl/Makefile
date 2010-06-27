LIBRARY=libGP_SDL

HEADERS=GP_SDL.h \
	GP_SDL_backend.h \
	GP_SDL_context.h \
	GP_SDL_specific.h

EXTRA_DEPS=

OBJECTS=GP_SDL_circle.o \
	GP_SDL_context.o \
	GP_SDL_ellipse.o \
	GP_SDL_getpixel.o \
	GP_SDL_line.o \
	GP_SDL_rect.o \
	GP_SDL_setpixel.o \
	GP_SDL_text.o \
	GP_SDL_triangle.o

.PHONY: library tests clean tar install

all: library tests

library: $(LIBRARY)

tests:
	cd tests && $(MAKE) all

$(LIBRARY): $(LIBRARY).a $(LIBRARY).so

$(OBJECTS): GP_SDL.h

#############################################################################
# Specific dependencies.
#############################################################################

GP_SDL_circle.o: GP_SDL_circle.c \
		../../generic/circle_generic.c \
		../../generic/fill_circle_generic.c

GP_SDL_ellipse.o: GP_SDL_ellipse.c \
		../../generic/ellipse_generic.c \
		../../generic/fill_ellipse_generic.c

GP_SDL_line.o: GP_SDL_line.c \
		../../generic/line_generic.c \
		../../generic/hline_generic.c \
		../../generic/vline_generic.c

GP_SDL_text.o: GP_SDL_text.c \
		../../generic/text_generic.c

GP_SDL_triangle.o: GP_SDL_triangle.c \
		../../generic/triangle_generic.c \
		../../generic/fill_triangle_generic.c

#############################################################################
# Building rules.
#############################################################################

CFLAGS=-W -Wall -O2 -fPIC -I../.. -I../../core

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) $< -c -o $@ 

$(LIBRARY).a: $(OBJECTS) $(EXTRA_DEPS)
	ar crus $@ $^

$(LIBRARY).so: $(OBJECTS) $(EXTRA_DEPS)
	$(CC) -fPIC -dPIC --shared -Wl,-soname -Wl,$@.0 $(CFLAGS) $^ -o $@
	ln -sf $@ $@.0

#############################################################################
# Installation, cleanup, and packing.
#############################################################################

HEADER_LOC=/usr/include/
LIB_LOC=/usr/lib/

install:
	install -m 775 -d $(HEADER_LOC)GP/
	install -m 664 *.h $(HEADER_LOC)GP/
	install -m 775 -d $(HEADER_LOC)GP/backends/
	install -m 664 backends/*.h $(HEADER_LOC)GP/backends/
	install -m 664 *.so *.so.0 *.a $(LIB_LOC)

clean:
	rm -f $(OBJECTS)
	rm -f $(LIBRARY).a $(LIBRARY).so $(LIBRARY).so.0
	cd tests && $(MAKE) clean

tar: clean
	cd .. && tar cjf gfxprim-`date +%Y-%b-%d`.tar.bz2 gfxprim
