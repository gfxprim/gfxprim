TOPDIR=../..

include $(TOPDIR)/include.mk

# Core
core/gfxprim_core_c.py core/gfxprim_core_wrap.c: $(TOPDIR)/include/core/gfxprim_core.swig
	swig -python -Wall -I/usr/include/  \
	  $(TOPDIR)/include/core/gfxprim_core.swig

$(TOPDIR)/build/_gfxprim_core_c.so: core/gfxprim_core_wrap.c
	gcc -shared core/gfxprim_core_wrap.c -L $(TOPDIR)/build/ -I /usr/include/python2.6/ -I$(TOPDIR)/include/ \
	    -fPIC -Wall -lGP -lpng -ljpeg -lm -ldl -o $(TOPDIR)/build/_gfxprim_core_c.so


all: $(TOPDIR)/build/_gfxprim_core_c.so

