TOPDIR=../..

CSOURCES=$(shell echo *.test.c) core_tests.gen.c

INCLUDE=core
LDLIBS+=-lGP -L$(TOPDIR)/build/ -lcheck

APPS=core_tests

include $(TOPDIR)/include.mk
include $(TOPDIR)/app.mk

.PHONY: run

TEST_SRCS=$(wildcard *.test.c)
TEST_OBJS=$(patsubst %.c,%.o,$(TEST_SRCS))
TESTS=core_tests
CLEAN+=core_tests.gen.c

run: $(TESTS)
	for test in $(TESTS); do LD_LIBRARY_PATH=../../build ./"$$test" -v ; done

core_tests.gen.c: $(TEST_SRCS) find_tests.py
ifdef VERBOSE
	python find_tests.py -c $@ $(TEST_SRCS)
else
	@echo "GEN (python) $@"
	@python find_tests.py -c $@ $(TEST_SRCS)
endif

core_tests: $(TEST_OBJS) core_tests.gen.o
