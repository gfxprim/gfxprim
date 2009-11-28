LIBRARY=libgfxprim
OBJECTS=basic_colors.o circle.o clear.o ellipse.o getpixel.o line.o rect.o setpixel.o triangle.o
CFLAGS=-W -Wall -O2 -fPIC

all: $(LIBRARY)
	cd tests && $(MAKE) all

$(LIBRARY): $(LIBRARY).a $(LIBRARY).so

$(OBJECTS): GP.h

$(LIBRARY).a: $(OBJECTS) 
	ar crus $@ $^

$(LIBRARY).so:
	$(CC) --shared -Wl,-soname -Wl,$@.0 $(CFLAGS) $(OBJECTS) -o $@
	ln -s $@ $@.0

%.o: %.c
	$(CC) $(CFLAGS) $< -c -o $@ 

clean:
	rm -f $(OBJECTS)
	rm -f $(LIBRARY).a $(LIBRARY).so $(LIBRARY).so.0
	cd tests && $(MAKE) clean

tar: clean
	cd .. && tar cjf gfxprim-`date +%Y-%b-%d`.tar.bz2 gfxprim
