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
	./bdf2c.pl $(FONTS_DIR)/HaxorMedium-10-utf.bdf haxor_medium_10 haxor-medium-10 9 > gp_haxor_medium_10.c && \
	./bdf2c.pl $(FONTS_DIR)/HaxorMedium-11-utf.bdf haxor_medium_11 haxor-medium-11 10 > gp_haxor_medium_11.c && \
	./bdf2c.pl $(FONTS_DIR)/HaxorMedium-12-utf.bdf haxor_medium_12 haxor-medium-12 11 > gp_haxor_medium_12.c && \
	HIRAGANA=1 KATAKANA=1 GREEK=1 CYRILIC=1 ./bdf2c.pl $(FONTS_DIR)/HaxorNarrow-15-utf.bdf haxor_narrow_15 haxor-narrow-15 14 > gp_haxor_narrow_15.c && \
	GREEK=1 CYRILIC=1 ./bdf2c.pl $(FONTS_DIR)/HaxorNarrow-16-utf.bdf haxor_narrow_16 haxor-narrow-16 15 > gp_haxor_narrow_16.c && \
	GREEK=1 CYRILIC=1 ./bdf2c.pl $(FONTS_DIR)/HaxorNarrow-17-utf.bdf haxor_narrow_17 haxor-narrow-17 16 > gp_haxor_narrow_17.c && \
	KATAKANA=1 GREEK=1 CYRILIC=1 ./bdf2c.pl $(FONTS_DIR)/HaxorNarrow-18-utf.bdf haxor_narrow_18 haxor-narrow-18 17 > gp_font_haxor_narrow_18.c && \
	CYRILIC=1 ./bdf2c.pl $(FONTS_DIR)/HaxorSquare-8x8-utf.bdf square square > gp_haxor_square_8x8.c
