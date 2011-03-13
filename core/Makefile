LIBRARY=libGP_core
SOURCES=$(wildcard *.c)
OBJECTS=$(SOURCES:.c=.o)
HEADERS=$(wildcard *.h)
ALGORITHMS=$(wildcard ../algo/*.algo.c)
CFLAGS=-W -Wall -O2 -fPIC -I../

HEADER_LOC=/usr/include/
LIB_LOC=/usr/lib/

.PHONY: all clean

all: $(LIBRARY)
	cd tests && $(MAKE) all

$(LIBRARY): $(LIBRARY).a $(LIBRARY).so

$(OBJECTS): $(HEADERS) $(ALGORITHMS)

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
	cd tests && $(MAKE) clean


GENERATORS=$(wildcard gen/*.py)

GP_Pixel_Scale.gen.h: $(GENERATORS)
	python gen/make_GP_Pixel_Scale.py GP_Pixel_Scale.gen.h 

GP_Pixel.gen.c GP_Pixel.gen.h: $(GENERATORS)
	python gen/make_GP_Pixel.py GP_Pixel.gen.h GP_Pixel.gen.c

generated: GP_Pixel_Scale.gen.h GP_Pixel.gen.c GP_Pixel.gen.h
