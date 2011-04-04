LIBRARY=../libGP_core.a
LDFLAGS=-L../ -lGP_core

INCLUDE=-I../

CFLAGS=$(INCLUDE) -ggdb -W -Wall -O -std=gnu99

TESTS=core_tests

# General stuff

.PHONY: all clean run

all: $(TESTS)

clean:
	rm -f *.o *.tmp
	rm -f $(TESTS)
	rm -f core_tests.gen.c

run: $(TESTS)
	for test in $(TESTS); do LD_LIBRARY_PATH=../ ./"$$test" -v ; done

# Check tests

TEST_SRCS=$(wildcard *.test.c)
TEST_OBJS=$(patsubst %.c,%.o,$(TEST_SRCS))

core_tests.gen.c: $(TEST_SRCS) find_tests.py
	python find_tests.py -c $@ $(TEST_SRCS)

core_tests: $(TEST_OBJS) core_tests.o core_tests.gen.o
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LIBRARY) -lcheck -o $@
