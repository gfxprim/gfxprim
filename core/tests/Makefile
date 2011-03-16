LIBRARY=../libGP_core.a
LDFLAGS=-L../ -lGP_core

INCLUDE=-I../
# Some warnings are triggered only with -O2
# thuss added here
CFLAGS=$(INCLUDE) -ggdb -W -Wall -O -std=gnu99

GPTESTS=GP_color_type_test GP_font_save GP_palette_test
TESTS=$(GPTESTS) core_tests

# General stuff

.PHONY: all clean run

all: $(TESTS)

clean:
	rm -f *.o *.tmp
	rm -f $(TESTS)

run: $(TESTS)
	for test in $(TESTS); do LD_LIBRARY_PATH=../ ./"$$test" ; done

# Non-check tests

$(GPTESTS): %: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LIBRARY) -o $@


# Check tests

TEST_SRCS=$(wildcard *.test.c)
TEST_OBJS=$(patsubst %.c,%.o,$(TEST_SRCS))

core_tests: $(TEST_OBJS) core_tests.o
	 $(CC) $(CFLAGS) $(LDFLAGS) $^ $(LIBRARY) -lcheck -o $@
	

