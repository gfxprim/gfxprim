TOPDIR=../..

CSOURCES=$(shell echo *.c)

INCLUDE=core gfx SDL backends
LDLIBS+=-lGP -L$(TOPDIR)/build/ -lGP_SDL -lSDL

ifeq ($(HAVE_LIBSDL),yes)
APPS=pixeltest fileview fonttest linetest randomshapetest shapetest sierpinsky\
     symbolstest textaligntest trianglefps input blittest subcontext showimage\
     aatest mixpixeltest
endif

include $(TOPDIR)/pre.mk
include $(TOPDIR)/app.mk
include $(TOPDIR)/post.mk
