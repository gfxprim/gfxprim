#
# Comment one of these lines.
#
# First one triggers static linking.
#
# Second one triggers dynamic.
#
LIBRARY=../libGP.a
#LDFLAGS=-lGP -lSDL -L..

INCLUDE=-I..
# Some warnings are triggered only with -O2
# thuss added here
CFLAGS=$(INCLUDE) -ggdb -W -Wall -O2
SOURCES=$(shell ls *.c)
TESTS=$(SOURCES:.c=)

all: $(TESTS)

%: %.c $(LIBRARY)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LIBRARY) -o $@ -lSDL

clean:
	rm -f *.o
	rm -f $(TESTS)

