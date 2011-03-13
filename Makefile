HEADER_LOC=/usr/include/
LIB_LOC=/usr/lib/

.PHONY: all core sdl drivers loaders install filters clean tar

all: core sdl drivers loaders filters

drivers:
	cd drivers && $(MAKE) all

core:
	cd core && $(MAKE) all

sdl: core
	cd targets/sdl && $(MAKE) all

loaders:
	cd loaders && $(MAKE) all

filters:
	cd filters && $(MAKE) all

install:
	# core library
	install -m 775 -d $(HEADER_LOC)GP/
	install -m 664 core/*.h $(HEADER_LOC)GP/
	install -m 664 core/*.so core/*.so.0 core/*.a $(LIB_LOC)
	# bitmap loaders
	install -m 775 -d $(HEADER_LOC)GP/loaders/
	install -m 664 loaders/*.h $(HEADER_LOC)GP/loaders/
	install -m 664 loaders/*.so loaders/*.so.0 loaders/*.a $(LIB_LOC)
	# context filters
	install -m 775 -d $(HEADER_LOC)GP/filters/
	install -m 664 filters/*.h $(HEADER_LOC)GP/filters/
	install -m 664 filters/*.so filters/*.so.0 filters/*.a $(LIB_LOC)
	# sdl target	
	install -m 775 -d $(HEADER_LOC)GP/SDL/
	install -m 664 targets/sdl/*.h $(HEADER_LOC)GP/SDL/
	install -m 664 targets/sdl/*.so targets/sdl/*.so.0 targets/sdl/*.a $(LIB_LOC)

clean:
	cd core && $(MAKE) clean
	cd targets/sdl && $(MAKE) clean
	cd drivers && $(MAKE) clean
	cd loaders && $(MAKE) clean
	cd filters && $(MAKE) clean

tar: clean
	cd .. && tar cjf gfxprim-`date +%Y-%b-%d-%HH%MM`.tar.bz2 gfxprim
