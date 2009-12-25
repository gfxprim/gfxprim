LIBRARY=../../libGP.a ../engine/*.a

INCLUDE=-I../..
CFLAGS=$(INCLUDE) -ggdb -W -Wall -O2
SOURCES=$(shell ls *.c)
TESTS=$(SOURCES:.c=)

all: $(TESTS)

%: %.c $(LIBRARY)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LIBRARY) -o $@ -lSDL

clean:
	rm -f *.o
	rm -f $(TESTS)

