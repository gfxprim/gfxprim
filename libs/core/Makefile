TOPDIR=../..
GENSOURCES=GP_Pixel.gen.c
GENHEADERS=GP_Pixel.gen.h
CSOURCES=$(filter-out $(wildcard *.gen.c),$(wildcard *.c))
LIBNAME=core

include $(TOPDIR)/gen.mk
include $(TOPDIR)/include.mk
include $(TOPDIR)/lib.mk

GP_Pixel.gen.c $(INCLUDE_PREFIX)GP_Pixel.gen.h: gen/make_GP_Pixel.py
	python gen/make_GP_Pixel.py $(INCLUDE_PREFIX)GP_Pixel.gen.h GP_Pixel.gen.c
