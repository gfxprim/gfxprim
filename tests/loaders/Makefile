TOPDIR=../..
include $(TOPDIR)/pre.mk

CSOURCES=loaders_suite.c PNG.c PBM.c PGM.c PPM.c
GENSOURCES=SaveLoad.gen.c

APPS=loaders_suite PNG PBM PGM PPM SaveLoad.gen

include ../tests.mk

include $(TOPDIR)/gen.mk
include $(TOPDIR)/app.mk
include $(TOPDIR)/post.mk
