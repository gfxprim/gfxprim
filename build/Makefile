LIB_OBJECTS=$(shell ./get_objs.sh)

all: libGP.a libGP.so libGP.so.0

libGP.a: $(LIB_OBJECTS)
libGP.so: $(LIB_OBJECTS)
libGP.so.0: $(LIB_OBJECTS)

rebuild: all

clean:
ifdef VERBOSE
	rm -rf libGP.a libGP.so libGP.so.0 .lock
else
	@echo "RM   libGP.a libGP.so libGP.so.0 .lock"
	@rm -rf libGP.a libGP.so libGP.so.0 .lock
endif

libGP.a:
ifdef VERBOSE
	$(AR) rcs libGP.a $(LIB_OBJECTS)
else
	@echo "AR   libGP.a"
	@$(AR) rcs libGP.a $(LIB_OBJECTS)
endif

libGP.so:
ifdef VERBOSE
	$(CC) -fPIC -dPIC --shared -Wl,-soname -Wl,libGP.so.0 $(LIB_OBJECTS) -o libGP.so
else
	@echo "LD   libGP.so"
	@$(CC) -fPIC -dPIC --shared -Wl,-soname -Wl,libGP.so.0 $(LIB_OBJECTS) -o libGP.so
endif

libGP.so.0: libGP.so
ifdef VERBOSE
	rm -f libGP.so.0
	ln -s libGP.so libGP.so.0
else
	@echo "LN   libGP.so.0"
	@rm -f libGP.so.0
	@ln -s libGP.so libGP.so.0
endif
