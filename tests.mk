.PHONY: tests runtests

#
# List of test targets (testsuite incl. automatically)
#
ifndef TESTS
TESTS=
endif

#
# Testsuite with automated collection of tests
# All .test.c files are scraped for GP_TEST definitions
#

ifdef TESTSUITE
# a bit crude way to link with test minilibrary 
GP_TESTLIB_SRCS=$(wildcard ${TOPDIR}/tests/common/*.c)
CSOURCES+=${GP_TESTLIB_SRCS}

# generated suite creation code
TESTSUITE_GEN=collected_tests.gen.c # also fixed in the code generator
CLEAN+=${TESTSUITE_GEN}

${TESTSUITE_GEN}: $(filter-out ${TESTSUITE_GEN},${GENSOURCES}) ${GENHEADERS}
ifdef VERBOSE
	${PYTHON} ${TOPDIR}/pylib/bin/generate_collected_tests.py -t $(TEMPLATE_DIR) "." "$@"
else
	@echo "TSTS $@"
	@${PYTHON} ${TOPDIR}/pylib/bin/generate_collected_tests.py -t $(TEMPLATE_DIR) "." "$@"
endif


TESTSUITE_SRCS=$(wildcard *.test.c) ${GENSOURCES} ${GENHEADERS} ${GP_TESTLIB_SRCS} ${TESTSUITE_GEN}
INCLUDE+=../tests/common
TESTSUITE_OBJS=$(patsubst %.c,%.o,$(TESTSUITE_SRCS))
CLEAN+=${TESTSUITE} ${TESTSUITE_OBJS}
TESTS+=${TESTSUITE}

${TESTSUITE}: ${TESTSUITE_OBJS}
ifdef VERBOSE
	$(CC) $(CFLAGS) $(LDFLAGS) -Wl,--start-group $(LDLIBS) $^ -Wl,--end-group -o $@
else
	@echo "LD  $@"
	@$(CC) $(CFLAGS) $(LDFLAGS) -Wl,--start-group $(LDLIBS) $^ -Wl,--end-group -o $@
endif # VERBOSE

endif # TESTSUITE

tests: $(TESTS)

runtests: tests
	for test in $(TESTS); do LD_LIBRARY_PATH=../../build ./"$$test" -v ; done

# WARN: avoid double includion?
include $(TOPDIR)/gen.mk
