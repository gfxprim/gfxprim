LDFLAGS=-L../ -L../../core/ -lGP_core -lGP_drivers

INCLUDE=-I../ -I../../core/
# Some warnings are triggered only with -O2
# thuss added here
CFLAGS=$(INCLUDE) -ggdb -W -Wall -O2 -lm
SOURCES=$(wildcard *.c)
TESTS=$(SOURCES:.c=)

all: $(TESTS)

%: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(LIBRARY) $^ -o $@

clean:
	rm -f *.o
	rm -f $(TESTS)

