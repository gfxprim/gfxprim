LIB_OBJECTS=$(shell ./get_objs.sh)

all: libGP.a libGP.so libGP.so.0

clean:
ifdef VERBOSE
	rm -f libGP.a libGP.so libGP.so.0
else
	@echo "RM  libGP.a libGP.so libGP.so.0"
	@rm -f libGP.a libGP.so libGP.so.0
endif

libGP.a: $(LIB_OBJECTS)
	@. ./liblock.sh; spinlock .
ifdef VERBOSE
	$(AR) rcs libGP.a $(LIB_OBJECTS)
else
	@echo "AR  libGP.a"
	@$(AR) rcs libGP.a $(LIB_OBJECTS)
endif
	@. ./liblock.sh; spinunlock .

libGP.so: $(LIB_OBJECTS)
	@. ./liblock.sh; spinlock .
ifdef VERBOSE
	$(CC) -fPIC -dPIC --shared -Wl,-soname -Wl,libGP.so.0 $(LIB_OBJECTS) -o libGP.so
else
	@echo "LD  libGP.so"
	@$(CC) -fPIC -dPIC --shared -Wl,-soname -Wl,libGP.so.0 $(LIB_OBJECTS) -o libGP.so
endif
	@. ./liblock.sh; spinunlock .

libGP.so.0: libGP.so
ifdef VERBOSE
	ln -s libGP.so libGP.so.0
else
	@echo "LN  libGP.so.0"
	@ln -s libGP.so libGP.so.0
endif
