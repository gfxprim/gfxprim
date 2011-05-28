ifndef LIBNAME
$(error LIBNAME not defined, fix your library Makefile)
endif

LIB=libGP_$(LIBNAME)
LIBP=$(TOPDIR)/build/

#
# If set to yes, builds single library for directory
#
ifeq ($(BUILDLIB),yes)
all: $(LIBP)$(LIB).so $(LIBP)$(LIB).a
endif

ifeq ($(BUILDLIB),yes)
CLEAN+=$(LIBP)$(LIB).so $(LIBP)$(LIB).so.0 $(LIBP)$(LIB).a
endif

#
# Trigger libGP.XX library rebuild
#
all: $(OBJECTS)
	@$(MAKE) --no-print-directory -C $(TOPDIR)/build/

#
# Rules for single library
#
$(LIBP)$(LIB).so: $(OBJECTS)
ifdef VERBOSE
	rm -f $(LIBP)$(LIB).so.0
	ln -s $(LIB).so $(LIBP)$(LIB).so.0
	$(CC) -fPIC -dPIC --shared -Wl,-soname -Wl,$@.0 $(OBJECTS) -o $@
else
	@rm -f $(LIBP)$(LIB).so.0
	@ln -s $(LIB).so $(LIBP)$(LIB).so.0
	@echo "LD  $@"
	@$(CC) -fPIC -dPIC --shared -Wl,-soname -Wl,$@.0 $(OBJECTS) -o $@
endif

$(LIBP)$(LIB).a: $(OBJECTS)
ifdef VERBOSE
	$(AR) rcs $@ $(OBJECTS)
else
	@echo "AR  $@"
	@$(AR) rcs $@ $(OBJECTS)
endif

CLEAN+=$(OBJECTS)
