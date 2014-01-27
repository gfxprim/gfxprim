TOPDIR=../..
include $(TOPDIR)/pre.mk

CSOURCES=loaders_suite.c PNG.c PBM.c PGM.c PPM.c ZIP.c GIF.c IO.c PNM.c PCX.c
GENSOURCES=SaveLoad.gen.c SaveAbort.gen.c

APPS=loaders_suite PNG PBM PGM PPM PNM SaveLoad.gen SaveAbort.gen ZIP GIF PCX\
     IO

include ../tests.mk

include $(TOPDIR)/gen.mk
include $(TOPDIR)/app.mk
include $(TOPDIR)/post.mk
