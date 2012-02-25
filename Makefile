TOPDIR=.
SUBDIRS=include libs tests pylib demos build
include post.mk

libs: include
build: libs
tests: build
pylib: build
demos: build

distclean:
ifdef VERBOSE
	rm config.h config.gen.mk
	$(MAKE) clean
else
	@$(MAKE) clean
	@rm config.h config.gen.mk
endif

HEADER_LOC=/usr/include/
LIB_LOC=/usr/lib/

install:
ifdef VERBOSE
	./install.sh ""
else
	@./install.sh ""
endif

tar:
	$(MAKE) clean
	cd .. && tar cf gfxprim-`date +%Y-%b-%d-%HH%MM`.tar gfxprim
