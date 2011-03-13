LDFLAGS=-L../ -L../../core/ -L../../loaders/ -lGP_core -lGP_loaders -lGP_filters

INCLUDE=-I../ -I../../core/ -I../../loaders/
# Some warnings are triggered only with -O2
# thuss added here
CFLAGS=$(INCLUDE) -ggdb -W -Wall -O2 -lm
SOURCES=$(wildcard *.c)
TESTS=$(SOURCES:.c=)

all: $(TESTS)

%: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(LIBRARY) $^ -o $@

clean:
	rm -f *.o $(TESTS)
	rm -f test-mv.pgm test-mh.pgm test-cw.pgm test-ccw.pgm
