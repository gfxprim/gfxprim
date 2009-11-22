LIBRARY=../gfxprim.a
INCLUDE=-I..
# Some warnings are triggered only with -O2
# thuss added here
CFLAGS=$(INCLUDE) -ggdb -W -Wall -O2
SOURCES=$(shell ls *.c)
TESTS=$(SOURCES:.c=)

all: $(TESTS)

%: %.c $(LIBRARY)
	gcc $(CFLAGS) $^ $(LIBRARY) -o $@ -lSDL

clean:
	rm -f *.o
	rm -f $(TESTS)

