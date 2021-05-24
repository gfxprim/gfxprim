TOPDIR=..
-include $(TOPDIR)/config.gen.mk
SUBDIRS=grinder spiv particle ttf2img c_simple bogoman

ifeq ($(HAVE_JSON-C),yes)
SUBDIRS+=widgets
endif

include $(TOPDIR)/post.mk
