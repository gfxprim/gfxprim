TOPDIR=../..

CSOURCES=$(shell echo *.c)

INCLUDE=core gfx SDL backends
LDLIBS+=-lGP -L$(TOPDIR)/build/ -lGP_SDL -lSDL

APPS=pixeltest fileview fonttest linetest randomshapetest shapetest sierpinsky\
     symbolstest textaligntest trianglefps input blittest subcontext showimage\
     aatest mixpixeltest

include $(TOPDIR)/include.mk
include $(TOPDIR)/app.mk
