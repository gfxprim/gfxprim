#
# Most of the libraries just generate object files that
# are later linked in build into the libgfxprim.so
#
ifndef LIBNAME
$(error LIBNAME not defined, fix your library Makefile)
endif

ifeq ($(BUILDLIB),yes)
#
# Rules for single library, applied only when objects
# are not linked to the libgfxprim.so. This generates libgfxprim_$(LIBNAME).
#
include $(TOPDIR)/libver.mk
include $(TOPDIR)/config.mk

LIB_NAME=libgfxprim-$(LIBNAME)
BUILD_DIR=$(TOPDIR)/build/

STATIC_LIB=$(LIB_NAME).a
DYNAMIC_LIB=$(LIB_NAME).so.$(LIB_VERSION).$(LIB_RELEASE)
SONAME=$(LIB_NAME).so.$(LIB_MAJOR)
SYMLINKS=$(LIB_NAME).so.$(LIB_MAJOR) $(LIB_NAME).so

ifeq ($(static-libs),yes)
ALL+=$(BUILD_DIR)$(STATIC_LIB)
endif

ifeq ($(dynamic-libs),yes)
ALL+=$(BUILD_DIR)$(DYNAMIC_LIB) $(SYMLINKS)
endif

CLEAN+=$(LIBS) $(addprefix $(BUILD_DIR),$(SYMLINKS))

#
# OBJECTS are set in post.mk so we need to duplicate the values in
# OBJS here too to have correct dependencies
#
OBJS=$(CSOURCES:.c=.o)

$(BUILD_DIR)$(DYNAMIC_LIB): $(OBJS)
ifdef VERBOSE
	rm -f $@
	$(CC) -fPIC --shared -Wl,-soname -Wl,$(SONAME) $^ $(LDLIBS_$(LIBNAME)) -o $@
else
	@rm -f $(@)
	@echo "LD   $@"
	@$(CC) -fPIC --shared -Wl,-soname -Wl,$(SONAME) $^ $(LDLIBS_$(LIBNAME)) -o $@
endif

$(BUILD_DIR)$(STATIC_LIB): $(OBJS)
ifdef VERBOSE
	$(AR) rcs $@ $^
else
	@echo "AR   $@"
	@$(AR) rcs $@ $^
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
ifndef TOP_MAKE
ALL+=rebuild_lib

rebuild_lib:
	@$(MAKE) --no-print-directory -C $(TOPDIR)/build/
endif
endif
