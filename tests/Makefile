TOPDIR=..
include $(TOPDIR)/pre.mk

SUBDIRS=core framework loaders gfx filters input utils widgets
TEST_DIRS=$(filter-out framework, $(SUBDIRS))

$(TEST_DIRS): framework

include $(TOPDIR)/post.mk
