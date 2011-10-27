LDFLAGS=-L../../build/ -lGP -lpng -ldl -ljpeg
INCLUDE=-I../../include
# Some warnings are triggered only with -O2
# thuss added here
CFLAGS=$(INCLUDE) -ggdb -W -Wall -O2
SOURCES=$(wildcard *.c)
TESTS=$(SOURCES:.c=)

all: $(TESTS)

%: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(LIBRARY) $^ -o $@

clean:
	rm -f *.o
	rm -f $(TESTS)

