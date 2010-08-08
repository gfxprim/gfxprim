LIBRARY=libGP_SDL

HEADERS=GP_SDL.h \
	GP_SDL_Context.h

EXTRA_DEPS=

OBJECTS=GP_SDL_Context.o

.PHONY: library tests clean tar install

all: library tests

library: $(LIBRARY)

tests:
	cd tests && $(MAKE) all

$(LIBRARY): $(LIBRARY).a $(LIBRARY).so

$(OBJECTS): GP_SDL.h

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
