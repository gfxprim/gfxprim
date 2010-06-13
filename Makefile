HEADER_LOC=/usr/include/
LIB_LOC=/usr/lib/

.PHONY: all core sdl install clean tar

all: core sdl

core:
	cd core && $(MAKE) all
	ln -sf core/libGP_core.so .
	ln -sf core/libGP_core.so.0 .

sdl: core
	cd targets/sdl && $(MAKE) all

#install:
#	install -m 775 -d $(HEADER_LOC)GP/
#	install -m 664 *.h $(HEADER_LOC)GP/
#	install -m 775 -d $(HEADER_LOC)GP/backends/
#	install -m 664 backends/*.h $(HEADER_LOC)GP/backends/
#	install -m 664 *.so *.so.0 *.a $(LIB_LOC)

clean:
	rm -f *.o *.a *.so *.so.0
	cd benchmark && $(MAKE) clean
	cd core && $(MAKE) clean
	cd targets/sdl && $(MAKE) clean

tar: clean
	cd .. && tar cjf gfxprim-`date +%Y-%b-%d-%HH%MM`.tar.bz2 gfxprim
