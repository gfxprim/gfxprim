TOPDIR=.
SUBDIRS=libs tests
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
ifdef VERBOSE
	./install.sh "/tmp/"
else
	@./install.sh "/tmp/"
endif

tar:
	$(MAKE) clean
	cd .. && tar cjf gfxprim-`date +%Y-%b-%d-%HH%MM`.tar.bz2 gfxprim
