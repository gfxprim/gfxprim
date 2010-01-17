LIBRARY=../libGP_SDL.a
LDFLAGS=-lSDL -L.. -lGP_SDL

INCLUDE=-I..
# Some warnings are triggered only with -O2
# thuss added here
CFLAGS=$(INCLUDE) -ggdb -W -Wall -O2
SOURCES=$(shell ls *.c)
TESTS=$(SOURCES:.c=)

all: $(TESTS)

%: %.c $(LIBRARY)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LIBRARY) -o $@

clean:
	rm -f *.o
	rm -f $(TESTS)

