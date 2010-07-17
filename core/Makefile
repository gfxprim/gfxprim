LIBRARY=libGP_core

OBJECTS=GP_circle.o \
	GP_defaultfont.o \
	GP_fillcircle.o \
	GP_fillcolumn.o \
	GP_fillrow.o \
	GP_filltriangle.o \
	GP_hline.o \
	GP_line.o \
	GP_putpixel.o \
	GP_rect.o \
	GP_textmetric.o \
	GP_triangle.o \
	GP_vline.o \
	GP_writepixel.o

HEADERS=GP.h \
	GP_abort.h \
	GP_check.h \
	GP_circle.h \
	GP_context.h \
	GP_fillcircle.h \
	GP_fillcolumn.h \
	GP_fillrow.h \
	GP_filltriangle.h \
	GP_font.h \
	GP_hline.h \
	GP_line.h \
	GP_putpixel.h \
	GP_rect.h \
	GP_swap.h \
	GP_textstyle.h \
	GP_textmetric.h \
	GP_triangle.h \
	GP_vline.h \
	GP_writepixel.h

CFLAGS=-ggdb -W -Wall -O2 -fPIC

HEADER_LOC=/usr/include/
LIB_LOC=/usr/lib/

.PHONY: all clean tar $(LIBRARY) sdl

all: $(LIBRARY)

$(LIBRARY): $(LIBRARY).a $(LIBRARY).so

$(OBJECTS): $(HEADERS)

$(LIBRARY).a: $(OBJECTS) 
	ar crus $@ $^

$(LIBRARY).so: $(OBJECTS)
	$(CC) -fPIC -dPIC --shared -Wl,-soname -Wl,$@.0 $(CFLAGS) $^ -o $@
	ln -sf $@ $@.0

%.o: %.c
	$(CC) $(CFLAGS) $< -c -o $@ 

clean:
	rm -f $(LIBRARY).a $(LIBRARY).so $(LIBRARY).so.0
	rm -f *.o

