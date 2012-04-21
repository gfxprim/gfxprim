#!/usr/bin/env python

import sys

import pygtk
pygtk.require('2.0')
import gtk

import gfxprim.core as core
import gfxprim.loaders as loaders
import gfxprim.filters as filters

class HelloWorld:
    def delete_event(self, widget, event, data=None):
        return False

    def destroy(self, widget, data=None):
        gtk.main_quit()

    def __init__(self):
        self.window = gtk.Window(gtk.WINDOW_TOPLEVEL)
        self.window.set_title("PyGTK Example")
	self.window.connect("delete_event", self.delete_event)
        self.window.connect("destroy", self.destroy)
        self.window.set_border_width(1)
        
        self.image = gtk.Image();
        self.window.add(self.image)
        
        self.window.show()
        self.image.show()
	
        img = loaders.LoadImage(sys.argv[2], None)
        self.pixmap = gtk.gdk.Pixmap(None, img.w, img.h, 24)
        self.gc = gtk.gdk.Drawable.new_gc(self.pixmap)

        img2 = img.Convert(core.core_c.GP_PIXEL_BGR888);

        self.pixmap.draw_rgb_image(self.gc, 0, 0, img2.w, img2.h,
                                   gtk.gdk.RGB_DITHER_NONE,
                                   img2._hacky_hacky_pixels_buffer())

        self.image.set_from_pixmap(self.pixmap, None)


    def main(self):
        gtk.main()

if __name__ == "__main__":
    hello = HelloWorld()
    hello.main()
