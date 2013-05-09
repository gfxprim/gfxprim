TOPDIR=.

include $(TOPDIR)/pre.mk

TOP_MAKE=1

ifneq ($(MAKECMDGOALS),install)
SUBDIRS=include libs tests pylib demos build
else
SUBDIRS=pylib
endif

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
	./install.sh "$(prefix)" $(libdir) $(bindir) $(includedir)
else
	@./install.sh "$(prefix)" $(libdir) $(bindir) $(includedir)
endif

tar:
	$(MAKE) clean
	cd .. && tar cf gfxprim-`date +%Y-%b-%d-%HH%MM`.tar gfxprim

include $(TOPDIR)/post.mk
