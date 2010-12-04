HEADER_LOC=/usr/include/
LIB_LOC=/usr/lib/

.PHONY: all core sdl drivers install clean tar

all: core sdl drivers

drivers:
	cd drivers && $(MAKE) all

core:
	cd core && $(MAKE) all
	ln -sf core/libGP_core.so .
	ln -sf core/libGP_core.so.0 .

sdl: core
	cd targets/sdl && $(MAKE) all

install: all
	# core library
	install -m 775 -d $(HEADER_LOC)GP/
	install -m 664 core/*.h $(HEADER_LOC)GP/
	install -m 664 core/*.so core/*.so.0 core/*.a $(LIB_LOC)
	# sdl target	
	install -m 775 -d $(HEADER_LOC)GP/SDL/
	install -m 664 targets/sdl/*.h $(HEADER_LOC)GP/SDL/
	install -m 664 targets/sdl/*.so targets/sdl/*.so.0 targets/sdl/*.a $(LIB_LOC)

clean:
	rm -f *.o *.a *.so *.so.0
	cd benchmark && $(MAKE) clean
	cd core && $(MAKE) clean
	cd targets/sdl && $(MAKE) clean
	cd drivers && $(MAKE) clean

tar: clean
	cd .. && tar cjf gfxprim-`date +%Y-%b-%d-%HH%MM`.tar.bz2 gfxprim
