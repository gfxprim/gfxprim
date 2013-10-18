TOPDIR=.

include $(TOPDIR)/pre.mk

TOP_MAKE=1

ifneq ($(MAKECMDGOALS),install)
SUBDIRS=include libs tests pylib demos build
else
SUBDIRS=pylib demos
endif

libs: include
build: libs
tests: build
pylib: build
demos: build

GENFILES=config.h config.gen.mk gfxprim-config

distclean:
ifdef VERBOSE
	rm $(GENFILES)
else
	@echo "RM   $(GENFILES)"
	@rm $(GENFILES)
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
