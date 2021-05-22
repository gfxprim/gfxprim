TOPDIR=..
include $(TOPDIR)/pre.mk

SUBDIRS=core framework loaders gfx filters input utils

ifeq ($(HAVE_JSON-C),yes)
SUBDIRS+=widgets
endif

loaders: framework
gfx: framework
core: framework
filters: framework
input: framework
widgets: framework

include $(TOPDIR)/post.mk
