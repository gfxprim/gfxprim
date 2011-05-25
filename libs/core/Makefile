TOPDIR=../..
GENSOURCES=GP_Pixel.gen.c GP_Blit.gen.c
GENHEADERS=GP_Pixel_Scale.gen.h GP_Blit.gen.h GP_Pixel.gen.h
CSOURCES=$(filter-out $(wildcard *.gen.c),$(wildcard *.c))
LIBNAME=core

include $(TOPDIR)/gen.mk
include $(TOPDIR)/include.mk
include $(TOPDIR)/lib.mk

$(INCLUDE_PREFIX)GP_Pixel.gen.h GP_Pixel.gen.c: ${GENERATORS}
	${PYTHON} "${PYLIBSDIR}/gfxprim/core/make_GP_Pixel.py" \
		"$(INCLUDE_PREFIX)GP_Pixel.gen.h" "GP_Pixel.gen.c"

$(INCLUDE_PREFIX)GP_Blit.gen.h GP_Blit.gen.c: ${GENERATORS}
	${PYTHON} "${PYLIBSDIR}/gfxprim/core/make_GP_Blit.py" \
		"$(INCLUDE_PREFIX)GP_Blit.gen.h" "GP_Blit.gen.c"

$(INCLUDE_PREFIX)GP_Pixel_Scale.gen.h: ${GENERATORS}
	${PYTHON} "${PYLIBSDIR}/gfxprim/core/make_GP_Pixel_Scale.py" \
		"$(INCLUDE_PREFIX)GP_Pixel_Scale.gen.h"

