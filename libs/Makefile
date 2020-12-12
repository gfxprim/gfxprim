TOPDIR=..
SUBDIRS=core utils gfx text loaders filters input backends grabbers

include $(TOPDIR)/config.mk

ifeq ($(HAVE_JSON-C),yes)
SUBDIRS+=widgets
endif

include $(TOPDIR)/post.mk
