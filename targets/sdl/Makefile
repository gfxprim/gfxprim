LIBRARY=libGP_SDL

HEADERS=GP_SDL.h \
	GP_SDL_backend.h

EXTRA_DEPS=../../libGP.a

OBJECTS=GP_SDL_circle.o \
	GP_SDL_getpixel.o \
	GP_SDL_line.o \
	GP_SDL_setpixel.o \
	GP_SDL_triangle.o

all: $(LIBRARY)
#	cd tests && $(MAKE) all
#	cd benchmark && $(MAKE) all

$(LIBRARY): $(LIBRARY).a $(LIBRARY).so

$(OBJECTS): GP_SDL.h

#############################################################################
# Building rules.
#############################################################################

CFLAGS=-W -Wall -O2 -fPIC -I../..

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) $< -c -o $@ 

$(LIBRARY).a: $(OBJECTS) $(EXTRA_DEPS)
	ar crus $@ $^

$(LIBRARY).so: $(OBJECTS) $(EXTRA_DEPS)
	$(CC) -fPIC -dPIC --shared -Wl,-soname -Wl,$@.0 $(CFLAGS) $(OBJECTS) -o $@
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
#	cd tests && $(MAKE) clean
#	cd benchmark && $(MAKE) clean

tar: clean
	cd .. && tar cjf gfxprim-`date +%Y-%b-%d`.tar.bz2 gfxprim
