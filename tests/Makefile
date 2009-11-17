LIBRARY=../gfxprim.a
INCLUDE=-I..
CFLAGS=$(INCLUDES)
SOURCES=$(shell ls *.c)
TESTS=$(SOURCES:.c=)

all: $(TESTS)

%: %.c $(LIBRARY)
	gcc $(INCLUDE) $^ $(LIBRARY) -o $@ -lSDL

clean:
	rm -f *.o
	rm -f $(TESTS)

