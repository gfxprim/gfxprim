COREDIR=`pwd`
LIBRARY=libGP_core
SOURCES=$(wildcard *.c) $(wildcard ../backends/*.c)
OBJECTS=$(SOURCES:.c=.o)
HEADERS=$(wildcard *.h) $(wildcard ../backends/*.h)
ALGORITHMS=$(wildcard ../algo/*.algo.h)
CFLAGS=-W -Wall -O2 -fPIC -I$(COREDIR) -I$(COREDIR)/../

HEADER_LOC=/usr/include/
LIB_LOC=/usr/lib/

.PHONY: all clean tests

all: $(LIBRARY)

tests: $(LIBRARY)
	cd tests && $(MAKE) all

$(LIBRARY): $(LIBRARY).a $(LIBRARY).so

$(OBJECTS): $(HEADERS) $(ALGORITHMS)

$(LIBRARY).a: $(OBJECTS) 
	ar crus $@ $^

$(LIBRARY).so: $(OBJECTS)
	$(CC) -fPIC -dPIC -lm -ldl --shared -Wl,-soname -Wl,$@.0 $(CFLAGS) $^ -o $@
	ln -sf $@ $@.0

%.o: %.c $(ALGORITHMS)
	$(CC) $(CFLAGS) $< -c -o $@ 

clean:
	rm -f $(LIBRARY).a $(LIBRARY).so $(LIBRARY).so.0
	rm -f *.o
	rm -f ../backends/*.o
	cd tests && $(MAKE) clean

