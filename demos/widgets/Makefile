TOPDIR=../..
include $(TOPDIR)/pre.mk

CSOURCES=$(shell echo *.c)

INCLUDE=
LDLIBS+=$(shell $(TOPDIR)/gfxprim-config --libs --libs-widgets)

APPS=show_layout clock todo pixmap_example example grid_move_widget tabs_example \
     markup_example switch_example overlay_example showimage scroll_area_example \
     datetime login_example example2 file_open

clock: LDLIBS+=-lm
showimage: LDLIBS+=-lgfxprim-loaders

include $(TOPDIR)/app.mk
include $(TOPDIR)/post.mk
