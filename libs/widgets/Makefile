TOPDIR=../..
include $(TOPDIR)/pre.mk

CSOURCES=$(shell ls *.c)
LIBNAME=widgets
BUILDLIB=yes

dialog_file_open.json.h: dialog_file_open.json
	@echo "J2H  $@"
	@./json2h $< > $@

include $(TOPDIR)/lib.mk
include $(TOPDIR)/post.mk
