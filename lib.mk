#
# Most of the libraries just generate object files that
# are later linked in build into the libGP.so
#
ifndef LIBNAME
$(error LIBNAME not defined, fix your library Makefile)
endif

ifeq ($(BUILDLIB),yes)
#
# Rules for single library, applied only when objects
# are not linked to the libGP.so. This generates libGP_$(LIBNAME).
#

include $(TOPDIR)/libver.mk

LIB_NAME=libGP_$(LIBNAME)
BUILD_DIR=$(TOPDIR)/build/

STATIC_LIB=$(LIB_NAME).$(LIB_VERSION).a
DYNAMIC_LIB=$(LIB_NAME).so.$(LIB_VERSION).$(LIB_RELEASE)
SONAME=$(LIB_NAME).so.$(LIB_MAJOR)
SYMLINKS=$(LIB_NAME).so.$(LIB_MAJOR) $(LIB_NAME).so

LIBS=$(BUILD_DIR)$(STATIC_LIB) $(BUILD_DIR)$(DYNAMIC_LIB)

ALL+=$(LIBS) $(SYMLINKS)
CLEAN+=$(LIBS) $(addprefix $(BUILD_DIR),$(SYMLINKS))

# 
# OBJECTS are set in post.mk so we need to set it here too to have
# correct dependencies we don't care that they are overwritten laten
#
OBJECTS=$(CSOURCES=.c=.o)

$(BUILD_DIR)$(DYNAMIC_LIB): $(OBJECTS)
ifdef VERBOSE
	rm -f $@
	$(CC) -fPIC --shared -Wl,-soname -Wl,$(SONAME) $(OBJECTS) -o $@
else
	@rm -f $(@)
	@echo "LD   $@"
	@$(CC) -fPIC --shared -Wl,-soname -Wl,$(SONAME) $(OBJECTS) -o $@
endif

$(BUILD_DIR)$(STATIC_LIB): $(OBJECTS)
ifdef VERBOSE
	$(AR) rcs $@ $(OBJECTS)
else
	@echo "AR   $@"
	@$(AR) rcs $@ $(OBJECTS)
endif

$(SYMLINKS): $(BUILD_DIR)$(DYNAMIC_LIB)
ifdef VERBOSE
	rm -f $(BUILD_DIR)$@
	cd $(BUILD_DIR) && ln -s $(DYNAMIC_LIB) $@
else
	@echo "LN   $@"
	@rm -f $(BUILD_DIR)$@
	@cd $(BUILD_DIR) && ln -s $(DYNAMIC_LIB) $@
endif

else
#
# If we are not executed from the top Makefile, trigger
# libGP.so rebuild.
#
include $(TOPDIR)/config.mk

ifndef TOP_MAKE
ALL+=rebuild_lib

rebuild_lib:
	@$(MAKE) --no-print-directory -C $(TOPDIR)/build/
endif
endif
