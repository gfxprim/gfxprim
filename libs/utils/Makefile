TOPDIR=../..
include $(TOPDIR)/pre.mk

CSOURCES=$(filter-out $(wildcard *.gen.c),$(wildcard *.c))
LIBNAME=core

#ifeq ($(wildcard $(os)),$(os))
CSOURCES+=$(wildcard $(os)/*.c)
#endif

#
# Regenerate gp_utf_tables.h from the latest Unicode data.
#
gen_utf_tables:
	curl -sS -o ./UnicodeData.txt https://unicode.org/Public/UCD/latest/ucd/UnicodeData.txt
	./gen_utf_tables.py ./UnicodeData.txt > gp_utf_tables.h
	rm ./UnicodeData.txt

include $(TOPDIR)/lib.mk
include $(TOPDIR)/post.mk
