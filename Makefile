LIBRARY=libGP

OBJECTS=basic_colors.o \
	circle.o clear.o \
	default_font.o \
	ellipse.o \
	text.o \
	getpixel.o \
	line.o \
	rect.o \
	setpixel.o \
	triangle.o

CFLAGS=-W -Wall -O2 -fPIC

HEADER_LOC=/usr/include/
LIB_LOC=/usr/lib/

all: $(LIBRARY)
	cd tests && $(MAKE) all
	cd benchmark && $(MAKE) all

$(LIBRARY): $(LIBRARY).a $(LIBRARY).so

$(OBJECTS): GP.h GP_gfx.h GP_clip_rect.h GP_colors.h GP_line.h GP_pixel.h GP_text.h

$(LIBRARY).a: $(OBJECTS) 
	ar crus $@ $^

$(LIBRARY).so:
	$(CC) -fPIC -dPIC --shared -Wl,-soname -Wl,$@.0 $(CFLAGS) $(OBJECTS) -o $@
	ln -s $@ $@.0

#############################################################################
# How object files from C codes are built. Many C files are including some
# template files; the resulting object is also dependent on the templates.
#############################################################################

%.o: %.c
	$(CC) $(CFLAGS) $< -c -o $@ 

setpixel.o: setpixel.c generic/setpixel_generic.c
	$(CC) $(CFLAGS) $< -c -o $@

line.o: line.c generic/line_generic.c generic/hline_generic.c generic/vline_generic.c
	$(CC) $(CFLAGS) $< -c -o $@

triangle.o: triangle.c generic/triangle_generic.c generic/fill_triangle_generic.c
	$(CC) $(CFLAGS) $< -c -o $@

#############################################################################
# Installation, cleanup, and packing.
#############################################################################

install:
	install -m 775 -d $(HEADER_LOC)GP/
	install -m 664 *.h $(HEADER_LOC)GP/
	install -m 664 *.so *.so.0 *.a $(LIB_LOC)

clean:
	rm -f $(OBJECTS)
	rm -f $(LIBRARY).a $(LIBRARY).so $(LIBRARY).so.0
	cd tests && $(MAKE) clean
	cd benchmark && $(MAKE) clean

tar: clean
	cd .. && tar cjf gfxprim-`date +%Y-%b-%d`.tar.bz2 gfxprim
