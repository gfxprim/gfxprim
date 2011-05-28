TOPDIR=.
SUBDIRS=libs tests pylib
include include.mk

#
# Make sure tests are build after library and 
# rebuild library before entering test just
# to be extra safe.
#
.PHONY: build

tests: build libs

build:
	@$(MAKE) --no-print-directory -C build clean
	@$(MAKE) --no-print-directory -C build

clean:
ifdef VERBOSE
	$(MAKE) -C build clean
else
	@echo "/build"
	@$(MAKE) --no-print-directory -C build clean
endif

HEADER_LOC=/usr/include/
LIB_LOC=/usr/lib/

install:
	# core library
#	install -m 775 -d $(HEADER_LOC)GP/
#	install -m 664 core/*.h $(HEADER_LOC)GP/
#	install -m 664 core/*.so core/*.so.0 core/*.a $(LIB_LOC)
	# bitmap loaders
#	install -m 775 -d $(HEADER_LOC)GP/loaders/
#	install -m 664 loaders/*.h $(HEADER_LOC)GP/loaders/
#	install -m 664 loaders/*.so loaders/*.so.0 loaders/*.a $(LIB_LOC)
	# context filters
#	install -m 775 -d $(HEADER_LOC)GP/filters/
#	install -m 664 filters/*.h $(HEADER_LOC)GP/filters/
#	install -m 664 filters/*.so filters/*.so.0 filters/*.a $(LIB_LOC)
	# sdl target	
#	install -m 775 -d $(HEADER_LOC)GP/SDL/
#	install -m 664 targets/sdl/*.h $(HEADER_LOC)GP/SDL/
#	install -m 664 targets/sdl/*.so targets/sdl/*.so.0 targets/sdl/*.a $(LIB_LOC)

tar:
	cd .. && tar cjf gfxprim-`date +%Y-%b-%d-%HH%MM`.tar.bz2 gfxprim
