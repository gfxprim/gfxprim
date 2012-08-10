TOPDIR=../..
include $(TOPDIR)/pre.mk

INCLUDE=core gfx SDL backends
LDLIBS+=-lGP -L$(TOPDIR)/build/ -lGP_SDL -lSDL

ifeq ($(HAVE_LIBSDL),yes)
CSOURCES=$(shell echo *.c)

APPS=pixeltest fonttest linetest randomshapetest\
     symbolstest textaligntest trianglefps blittest subcontext showimage\
     aatest mixpixeltest
endif

include $(TOPDIR)/app.mk
include $(TOPDIR)/post.mk
