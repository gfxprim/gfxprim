TOPDIR=../..
include $(TOPDIR)/pre.mk

CSOURCES=$(shell echo *.c)

INCLUDE=
LDLIBS+=$(shell $(TOPDIR)/gfxprim-config --libs --libs-widgets)

APPS=show_layout clock todo pixmap_example example grid_move_widget tabs_example \
     markup_example switch_example overlay_example showimage scroll_area_example \
     datetime login_example example2 file_dialogs tabs disk_free message_dialog \
     table_example focus choice log disable nested_dialogs choice2

clock: LDLIBS+=-lm
showimage: LDLIBS+=-lgfxprim-loaders
show_layout: LDLIBS+=-lgfxprim-loaders

include $(TOPDIR)/app.mk
include $(TOPDIR)/post.mk
