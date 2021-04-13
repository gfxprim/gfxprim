TOPDIR=../..
include $(TOPDIR)/pre.mk

CSOURCES=loaders_suite.c png.c pbm.c pgm.c ppm.c zip.c gif.c io.c pnm.c pcx.c\
         jpg.c loader.c data_storage.c exif.c line_convert.c ico.c

GENSOURCES=save_load.gen.c save_abort.gen.c

APPS=loaders_suite png pbm pgm ppm pnm save_load.gen save_abort.gen zip gif pcx\
     io jpg loader data_storage exif line_convert ico

include ../tests.mk

include $(TOPDIR)/gen.mk
include $(TOPDIR)/app.mk
include $(TOPDIR)/post.mk
