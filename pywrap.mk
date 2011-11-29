ifndef LIBNAME
$(error LIBNAME not defined, fix your library Makefile)
endif

include $(TOPDIR)/config.gen.mk

ifneq ($(SWIG),)

INCLUDES+=$(addprefix -I$(TOPDIR)/include/, $(INCLUDE))

all: _gfxprim_$(LIBNAME)_c.so

gfxprim_$(LIBNAME).c: gfxprim_$(LIBNAME).swig

gfxprim_$(LIBNAME).c: %.c: %.swig
ifdef VERBOSE
	$(SWIG) -python -Wall -I/usr/include/ $(INCLUDES) $<
	cp gfxprim_$(LIBNAME)_c.py ../../pylib/
else
	@echo "SWIG $(LIBNAME)"
	@$(SWIG) -python -Wall -I/usr/include/ $(INCLUDES) $<
	@cp gfxprim_$(LIBNAME)_c.py ../../pylib/
endif

_gfxprim_$(LIBNAME)_c.so: gfxprim_$(LIBNAME).c
ifdef VERBOSE
	$(CC) -fPIC -dPIC --shared -Wall -lGP -lpng -ljpeg -lm -ldl -o _gfxprim_$(LIBNAME)_c.so
else
	@echo "LD  $@"
	@$(CC) -fPIC -dPIC --shared -Wall -lGP -lpng -ljpeg -lm -ldl -o _gfxprim_$(LIBNAME)_c.so
endif

CLEAN+=gfxprim_$(LIBNAME).c gfxprim_$(LIBNAME)_c.py gfxprim_$(LIBNAME)_wrap.c _gfxprim_$(LIBNAME)_c.so
endif
