LIBRARY=libGP

OBJECTS=basic_colors.o \
	circle.o clear.o \
	default_font.o \
	ellipse.o \
	font.o \
	getpixel.o \
	line.o \
	rect.o \
	setpixel.o \
	triangle.o

CFLAGS=-W -Wall -O2

HEADER_LOC=/usr/include/
LIB_LOC=/usr/lib/

all: $(LIBRARY)
	cd tests && $(MAKE) all

$(LIBRARY): $(LIBRARY).a $(LIBRARY).so

$(OBJECTS): GP.h

$(LIBRARY).a: $(OBJECTS) 
	ar crus $@ $^

$(LIBRARY).so:
	$(CC) -fPIC -dPIC --shared -Wl,-soname -Wl,$@.0 $(CFLAGS) $(OBJECTS) -o $@
	ln -s $@ $@.0

%.o: %.c
	$(CC) $(CFLAGS) $< -c -o $@ 

install:
	install -m 775 -d $(HEADER_LOC)GP/
	install -m 664 *.h $(HEADER_LOC)GP/
	install -m 664 *.so *.so.0 *.a $(LIB_LOC)

clean:
	rm -f $(OBJECTS)
	rm -f $(LIBRARY).a $(LIBRARY).so $(LIBRARY).so.0
	cd tests && $(MAKE) clean

tar: clean
	cd .. && tar cjf gfxprim-`date +%Y-%b-%d`.tar.bz2 gfxprim
