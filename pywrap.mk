ifndef LIBNAME
$(error LIBNAME not defined, fix your library Makefile)
endif

include $(TOPDIR)/config.gen.mk

ifneq ($(SWIG),)

INCLUDES+=$(addprefix -I$(TOPDIR)/include/, $(INCLUDE))

all: _gfxprim_$(LIBNAME)_c.so gfxprim_$(LIBNAME)_c.py

gfxprim_$(LIBNAME)_wrap.c gfxprim_$(LIBNAME)_c.py: gfxprim_$(LIBNAME).swig
ifdef VERBOSE
	$(SWIG) -python -Wall -I/usr/include/ $(INCLUDES) $<
else # VERBOSE
	@echo "SWIG $(LIBNAME)"
	@$(SWIG) -python -Wall -I/usr/include/ $(INCLUDES) $<
endif # VERBOSE

_gfxprim_$(LIBNAME)_c.so: gfxprim_$(LIBNAME)_wrap.c
ifdef VERBOSE
	$(CC) gfxprim_$(LIBNAME)_wrap.c $(CFLAGS) $(LDFLAGS) -I$(PYTHON_INCLUDE) -dPIC --shared -lGP -L$(TOPDIR)/build/ -o _gfxprim_$(LIBNAME)_c.so
else # VERBOSE
	@echo "LD  $@"
	@$(CC) gfxprim_$(LIBNAME)_wrap.c $(CFLAGS) $(LDFLAGS) -I$(PYTHON_INCLUDE) -dPIC --shared -lGP -L$(TOPDIR)/build/ -o _gfxprim_$(LIBNAME)_c.so
endif # VERBOSE

endif # ifneq ($(SWIG),)

CLEAN+=gfxprim_$(LIBNAME)_wrap.c gfxprim_$(LIBNAME)_c.py _gfxprim_$(LIBNAME)_c.so
