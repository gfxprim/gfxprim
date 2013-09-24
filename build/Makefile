LIB_OBJECTS=$(shell ./get_objs.sh)

.SUFFIXES:

TOPDIR=../
include $(TOPDIR)/config.mk
include $(TOPDIR)/libver.mk

LIB_NAME=libGP

STATIC_LIB=$(LIB_NAME).$(LIB_VERSION).a
DYNAMIC_LIB=$(LIB_NAME).so.$(LIB_VERSION).$(LIB_RELEASE)
SONAME=$(LIB_NAME).so.$(LIB_MAJOR)
SYMLINKS=$(LIB_NAME).so.$(LIB_MAJOR) $(LIB_NAME).so

all: $(STATIC_LIB) $(DYNAMIC_LIB) $(SYMLINKS)

rebuild: all

distclean: clean

clean:
ifdef VERBOSE
	rm -rf $(STATIC_LIB) $(DYNAMIC_LIB) $(SYMLINKS)
else
	@echo "RM   $(STATIC_LIB) $(DYNAMIC_LIB) $(SYMLINKS)"
	@rm -rf $(STATIC_LIB) $(DYNAMIC_LIB) $(SYMLINKS)
endif

$(STATIC_LIB): $(LIB_OBJECTS)
ifdef VERBOSE
	$(AR) rcs $@ $^
else
	@echo "AR   $@"
	@$(AR) rcs $@ $^
endif

$(DYNAMIC_LIB): $(LIB_OBJECTS)
ifdef VERBOSE
	$(CC) -fPIC -dPIC --shared -Wl,-soname -Wl,$(SONAME) $^ -o $@
else
	@echo "LD   $@"
	@$(CC) -fPIC -dPIC --shared -Wl,-soname -Wl,$(SONAME) $^ -o $@
endif

$(SYMLINKS): $(DYNAMIC_LIB)
ifdef VERBOSE
	rm -f $@
	ln -s $< $@
else
	@echo "LN   $@"
	@rm -f $@
	@ln -s $< $@
endif
