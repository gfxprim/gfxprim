ifndef LIBNAME
$(error LIBNAME not defined, fix your library Makefile)
endif

#
# Trigger library rebuild
#
all: $(OBJECTS)
	@$(MAKE) --no-print-directory -C $(TOPDIR)/build/

CLEAN+=$(OBJECTS)
