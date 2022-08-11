TOPDIR=.

include $(TOPDIR)/pre.mk

TOP_MAKE=1

ifneq ($(MAKECMDGOALS),install)
SUBDIRS=include libs tests pylib demos
ifneq ($(MAKECMDGOALS),gen)
SUBDIRS+=build
endif
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
	rm -f $(GENFILES)
else
	@echo "RM   $(GENFILES)"
	@rm -f $(GENFILES)
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

afl:
	@if [ -z `which afl-gcc 2>/dev/null` ]; then \
		echo "ERROR: afl-gcc not installed!"; \
		exit 127; \
	fi
	CC=afl-gcc-fast ./configure
	$(MAKE) -C include
	$(MAKE) -C libs
	$(MAKE) afl -C tests/afl

include $(TOPDIR)/post.mk
