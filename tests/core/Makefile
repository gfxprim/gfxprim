TOPDIR=../..

include $(TOPDIR)/pre.mk

CSOURCES=pixmap.c pixel.c blit_clipped.c debug.c sub_pixmap_put_pixel.c

GENSOURCES+=write_pixel.gen.c get_put_pixel.gen.c convert.gen.c blit_conv.gen.c \
            convert_scale.gen.c get_set_bits.gen.c write_pixels2.gen.c

APPS=write_pixel.gen pixel pixmap get_put_pixel.gen convert.gen blit_conv.gen \
     convert_scale.gen get_set_bits.gen blit_clipped debug write_pixels2.gen \
     sub_pixmap_put_pixel

include ../tests.mk

include $(TOPDIR)/gen.mk
include $(TOPDIR)/app.mk
include $(TOPDIR)/post.mk
