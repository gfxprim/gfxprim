TOPDIR=../..
include $(TOPDIR)/pre.mk

CSOURCES=filter_mirror_h.c common.c linear_convolution.c

GENSOURCES=api_coverage.gen.c filters_compare.gen.c

APPS=filter_mirror_h api_coverage.gen filters_compare.gen linear_convolution

include ../tests.mk

filter_mirror_h: common.o

include $(TOPDIR)/gen.mk
include $(TOPDIR)/app.mk
include $(TOPDIR)/post.mk
