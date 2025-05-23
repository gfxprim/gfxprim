.SUFFIXES:

TOPDIR=../
include $(TOPDIR)/config.mk
include $(TOPDIR)/libver.mk

LIB_OBJECTS=$(shell ./get_objs.sh $(os))

LIB_NAME=libgfxprim

STATIC_LIB=$(LIB_NAME).a
DYNAMIC_LIB=$(LIB_NAME).so.$(LIB_VERSION).$(LIB_RELEASE)
SONAME=$(LIB_NAME).so.$(LIB_MAJOR)
SYMLINKS=$(LIB_NAME).so.$(LIB_MAJOR) $(LIB_NAME).so

ifeq ($(static-libs),yes)
LIBS+=$(STATIC_LIB)
endif

ifeq ($(dynamic-libs),yes)
LIBS+=$(DYNAMIC_LIB) $(SYMLINKS)
endif

all: $(LIBS)

INSTALL_PKGCONFIG=$(wildcard *.pc)
install: $(INSTALL_PKGCONFIG)

include $(TOPDIR)/install.mk

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
	$(CC) -fPIC -dPIC --shared -Wl,-soname -Wl,$(SONAME) $^ -lm -lrt $(LDLIBS_core) -o $@
else
	@echo "LD   $@"
	@$(CC) -fPIC -dPIC --shared -Wl,-soname -Wl,$(SONAME) $^ -lm -lrt $(LDLIBS_core) -o $@
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
