TOPDIR=../..
include $(TOPDIR)/pre.mk
include $(TOPDIR)/config.mk

GENSOURCES=gp_text.gen.c
CSOURCES=$(filter-out $(wildcard *.gen.c),$(wildcard *.c))
LIBNAME=text

include $(TOPDIR)/gen.mk
include $(TOPDIR)/lib.mk
include $(TOPDIR)/post.mk

ifeq ($(HAVE_FREETYPE),yes)
gp_free_type.dep: CFLAGS+=$(FREETYPE_CFLAGS)
gp_free_type.o: CFLAGS+=$(FREETYPE_CFLAGS)
endif

FONTS_DIR="../../../fonts/"

fonts:
	export LICENSE="SPDX-License-Identifier: LGPL-2.1-or-later" && \
	./bdf2c.pl $(FONTS_DIR)/HaxorNarrow-15-utf.bdf haxor_narrow_15 haxor-narrow-15 14 > gp_haxor_narrow_15.c && \
	./bdf2c.pl $(FONTS_DIR)/HaxorNarrow-16-utf.bdf haxor_narrow_16 haxor-narrow-16 15 > gp_haxor_narrow_16.c && \
	./bdf2c.pl $(FONTS_DIR)/HaxorNarrow-17-utf.bdf haxor_narrow_17 haxor-narrow-17 16 > gp_haxor_narrow_17.c && \
	./bdf2c.pl $(FONTS_DIR)/HaxorNarrow-18-utf.bdf haxor_narrow_18 haxor-narrow-18 17 > gp_font_haxor_narrow_18.c && \
	./bdf2c.pl $(FONTS_DIR)/HaxorSquare-8x8-utf.bdf square square > gp_haxor_square_8x8.c
