TOPDIR=../..

CSOURCES=$(shell echo *.c)

INCLUDE=core gfx sdl backends
LDLIBS+=-lGP -L$(TOPDIR)/build/ -lGP_sdl -lSDL

APPS=pixeltest fileview fonttest linetest randomshapetest shapetest sierpinsky\
     symbolstest textaligntest trianglefps

include $(TOPDIR)/include.mk
include $(TOPDIR)/app.mk
