#
# Most of the libraries just generate object files that
# are later linked in build into the libGP.so
#
ifndef LIBNAME
$(error LIBNAME not defined, fix your library Makefile)
endif

#
# Rules for single library, applied only when objects
# are not linked to the libGP.so. This generates libGP_$(LIBNAME).
#
ifeq ($(BUILDLIB),yes)
# BUILDLIB = yes

LIB=libGP_$(LIBNAME)
LIBP=$(TOPDIR)/build/

ALL+=$(LIBP)$(LIB).so $(LIBP)$(LIB).a
CLEAN+=$(LIBP)$(LIB).so $(LIBP)$(LIB).so.0 $(LIBP)$(LIB).a
OBJS=$(CSOURCES:.c=.o)


$(LIBP)$(LIB).so: $(OBJS)
ifdef VERBOSE
	rm -f $(LIBP)$(LIB).so.0
	cd $(LIBP) && ln -s $(LIB).so $(LIB).so.0
	$(CC) -fPIC -dPIC --shared -Wl,-soname -Wl,$(LIB).so.0 $(OBJECTS) -o $@
else
	@rm -f $(LIBP)$(LIB).so.0
	@cd $(LIBP) && ln -s $(LIB).so $(LIB).so.0
	@echo "LD  $@"
	@$(CC) -fPIC -dPIC --shared -Wl,-soname -Wl,$(LIB).so.0 $(OBJECTS) -o $@
endif

$(LIBP)$(LIB).a: $(OBJS)
ifdef VERBOSE
	$(AR) rcs $@ $(OBJECTS)
else
	@echo "AR  $@"
	@$(AR) rcs $@ $(OBJECTS)
endif

else
# BUILDLIB = no
include $(TOPDIR)/config.mk

ifeq ($(REBUILD_LIBGP),yes)
ALL+=rebuild_lib

rebuild_lib:
	@$(MAKE) --no-print-directory -C $(TOPDIR)/build/
endif
endif
