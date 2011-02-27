LIBRARY=libGP_filters
SOURCES=$(wildcard *.c)
OBJECTS=$(SOURCES:.c=.o)
HEADERS=$(wildcard *.h)
CFLAGS=-I../core/ -W -Wall -O2 -fPIC

.PHONY: all clean

all: $(LIBRARY)
	cd tests && $(MAKE) all

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
	cd tests && $(MAKE) clean
