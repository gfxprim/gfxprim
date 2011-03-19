LIBRARY=../libGP_core.a
LDFLAGS=-L../ -lGP_core

INCLUDE=-I../
# Some warnings are triggered only with -O2
# thuss added here
CFLAGS=$(INCLUDE) -ggdb -W -Wall -O -std=gnu99

TESTS=core_tests

# General stuff

.PHONY: all clean run

all: $(TESTS)

clean:
	rm -f *.o *.tmp
	rm -f $(TESTS)

run: $(TESTS)
	for test in $(TESTS); do LD_LIBRARY_PATH=../ ./"$$test" ; done

# Check tests

TEST_SRCS=$(wildcard *.test.c)
TEST_OBJS=$(patsubst %.c,%.o,$(TEST_SRCS))

core_tests: $(TEST_OBJS) core_tests.o
	 $(CC) $(CFLAGS) $(LDFLAGS) $^ $(LIBRARY) -lcheck -o $@
