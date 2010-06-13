LIBRARY=../libGP_SDL.a ../../../libGP_core.a
LDFLAGS=-lSDL -L.. -L../../../core -lGP_SDL -lGP_core

INCLUDE=-I.. -I../../../core
# Some warnings are triggered only with -O2
# thuss added here
CFLAGS=$(INCLUDE) -ggdb -W -Wall -O
SOURCES=$(shell ls *.c)
TESTS=$(SOURCES:.c=)

all: $(TESTS)

%: %.c $(LIBRARY)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LIBRARY) -o $@

clean:
	rm -f *.o
	rm -f $(TESTS)

