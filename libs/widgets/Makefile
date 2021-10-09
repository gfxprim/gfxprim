TOPDIR=../..
include $(TOPDIR)/pre.mk

CSOURCES=$(shell ls *.c)
LIBNAME=widgets
BUILDLIB=yes
DIALOGS=dialog_err.json.h dialog_info.json.h dialog_warn.json.h \
	dialog_question.json.h dialog_file_open.json.h dialog_file_save.json.h \
	dialog_input.json.h

$(DIALOGS): %.json.h: %.json
	@echo "J2H  $@"
	@./json2h $< > $@

include $(TOPDIR)/lib.mk
include $(TOPDIR)/post.mk
