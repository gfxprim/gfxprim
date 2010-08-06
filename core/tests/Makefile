LIBRARY=../libGP_core.a
LDFLAGS=-L../ -lGP_core

INCLUDE=-I../
# Some warnings are triggered only with -O2
# thuss added here
CFLAGS=$(INCLUDE) -ggdb -W -Wall -O
SOURCES=$(wildcard *.c)
TESTS=$(SOURCES:.c=)

all: $(TESTS)

%: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LIBRARY) -o $@

clean:
	rm -f *.o
	rm -f $(TESTS)

