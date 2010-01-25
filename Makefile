LIBRARY=libGP_common

OBJECTS=GP_default_font.o \
	GP_text_metric.o

HEADERS=GP_common.h \
	GP_font.h \
	GP_textstyle.h \
	GP_text_metric.h

CFLAGS=-W -Wall -O2 -fPIC

HEADER_LOC=/usr/include/
LIB_LOC=/usr/lib/

.PHONY: all clean tar $(LIBRARY) sdl

all: $(LIBRARY)

sdl: $(LIBRARY)
	cd targets/sdl && $(MAKE) all

$(LIBRARY): $(LIBRARY).a $(LIBRARY).so

$(OBJECTS): $(HEADERS)

$(LIBRARY).a: $(OBJECTS) 
	ar crus $@ $^

$(LIBRARY).so: $(OBJECTS)
	$(CC) -fPIC -dPIC --shared -Wl,-soname -Wl,$@.0 $(CFLAGS) $^ -o $@
	ln -sf $@ $@.0

%.o: %.c
	$(CC) $(CFLAGS) $< -c -o $@ 

#install:
#	install -m 775 -d $(HEADER_LOC)GP/
#	install -m 664 *.h $(HEADER_LOC)GP/
#	install -m 775 -d $(HEADER_LOC)GP/backends/
#	install -m 664 backends/*.h $(HEADER_LOC)GP/backends/
#	install -m 664 *.so *.so.0 *.a $(LIB_LOC)

clean:
	rm -f $(LIBRARY).a $(LIBRARY).so $(LIBRARY).so.0
	rm -f *.o
	cd tests && $(MAKE) clean
	cd benchmark && $(MAKE) clean
	cd targets/sdl && $(MAKE) clean

tar: clean
	cd .. && tar cjf gfxprim-`date +%Y-%b-%d`.tar.bz2 gfxprim
