TOPDIR=../..

CSOURCES=$(shell echo *.c)

INCLUDE=core gfx sdl backends
LDLIBS+=-lGP -L$(TOPDIR)/build/ -lSDL

APPS=pixeltest fileview fonttest linetest randomshapetest shapetest sierpinsky\
     symboltest textaligntest trianglefps

include $(TOPDIR)/include.mk
include $(TOPDIR)/app.mk


$(APPS): $(TOPDIR)/libs/sdl/*.o

