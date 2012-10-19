TOPDIR=../..
include $(TOPDIR)/pre.mk

INCLUDE=core gfx SDL backends
LDLIBS+=-lGP -L$(TOPDIR)/build/ -lGP_SDL -lSDL

ifeq ($(HAVE_LIBSDL),yes)
CSOURCES=$(shell echo *.c)

APPS=pixeltest fonttest randomshapetest\
     symbolstest textaligntest blittest subcontext\
     aatest mixpixeltest
endif

include $(TOPDIR)/app.mk
include $(TOPDIR)/post.mk
