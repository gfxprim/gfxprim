#
# Make no subdirs by default
#
ifndef SUBDIRS
SUBDIRS=
endif

.PHONY: $(SUBDIRS) all clean rebuild help doc install

all: $(SUBDIRS)
clean: $(SUBDIRS)
distclean: $(SUBDIRS)
rebuild: $(SUBDIRS)
install: $(SUBDIRS)

help:
	@echo "***        Available targets        ***"
	@echo ""
	@echo "help:      prints this help"
	@echo ""
	@echo "doc:       builds (only) the documentation"
	@echo ""
	@echo "clean:     cleans current directory and all subdirectories"
	@echo ""
	@echo "distclean: cleans all generated files including possible"
	@echo "           leftovers caused by deleted files"
	@echo ""
	@echo "all:       make current directory and all subdirectories"
	@echo ""
	@echo "rebuild:   does clean and all"
	@echo ""
	@echo "check:     do pre-commit check"
	@echo "           (currently checks for exported symbols)"
	@echo ""
	@echo "The default silent output could be turned off by defining"
	@echo "'VERBOSE' shell variable as 'VERBOSE=1 make'"
	@echo ""

doc:
	@cd $(TOPDIR)/doc && make

check:
	@cd $(TOPDIR)/build && ./check_symbols.sh

#
# Determine mode (eg do not generate anything if not in compile mode
#
COMPILE=no

ifeq ($(MAKECMDGOALS),all)
COMPILE=yes
endif

ifeq ($(MAKECMDGOALS),)
COMPILE=yes
endif

ifeq ($(MAKECMDGOALS),rebuild)
COMPILE=yes
endif

#
# 1. Generate and include dependencies for all C sources
# 2. Generate OBJECTS list from CSOURCES list
# 3. Adds OBJECTS to CLEAN list
#
ifdef CSOURCES
DEPFILES=$(subst .c,.dep,$(CSOURCES))
ifeq ($(COMPILE),yes)
-include $(DEPFILES)
endif
CLEAN+=$(subst .c,.dep,$(CSOURCES))
OBJECTS+=$(CSOURCES:.c=.o)
CLEAN+=$(OBJECTS)
endif

#
# Automatically include library headers
#
ifdef LIBNAME
INCLUDE+=$(LIBNAME)
endif

#
# If there was anything in INCLUDE list, create CFLAGS for each entry
#
ifdef INCLUDE
CFLAGS+=$(addprefix -I$(TOPDIR)/include/, $(INCLUDE))
endif

#
# Walk trought SUBDIRS, this code works even for -jX
#
$(SUBDIRS):
ifdef VERBOSE
	$(MAKE) -C $@ $(MAKECMDGOALS) TOP_MAKE=$(TOP_MAKE)
else
	@export CURSUBDIR="$$CURSUBDIR/$@" && echo "DIR  $$CURSUBDIR" &&\
	$(MAKE) --no-print-directory -C $@ $(MAKECMDGOALS) TOP_MAKE=$(TOP_MAKE)
endif

#
# Actual make rules
#
$(DEPFILES): %.dep: %.c
ifdef VERBOSE
	$(CC) -MM $(CFLAGS) $< -o $@
else
	@echo "DEP  -I(include $(INCLUDE)) $@"
	@$(CC) -MM $(CFLAGS) $< -o $@
endif

$(OBJECTS): %.o: %.c
ifdef VERBOSE
	$(CC) $(CFLAGS) -c $< -o $@
else
	@echo "CC   -I(include $(INCLUDE)) $@"
	@$(CC) $(CFLAGS) -c $< -o $@
endif

ifdef CLEAN

ifeq ($(MAKECMDGOALS),distclean)
CLEAN+=*.o *.dep *.gen.c *.gen.h
endif

distclean: clean

clean:
ifdef VERBOSE
	rm -f $(CLEAN)
else
	@echo "RM   $(CLEAN)"
	@rm -f $(CLEAN)
endif
endif

compile: $(DEPFILES) $(OBJECTS) $(ALL)

rebuild: clean
	@$(MAKE) --no-print-directory all
