#
# This is makefile rule for generating C sources from python generators
#
ifndef LIBNAME
$(error LIBNAME not defined, fix your library Makefile)
endif

#
# Headers goes into include/core/
#
INCLUDE_PREFIX=$(TOPDIR)/include/$(LIBNAME)/
RGENHEADERS=$(addprefix $(INCLUDE_PREFIX),$(GENHEADERS))

#
# Generate genfiles for generated sources
#
CSOURCES+=$(GENSOURCES)

#
# Make the targets build actually
#
all: $(RGENHEADERS)

#
# And clean them
#
CLEAN+=$(GENSOURCES) $(RGENHEADERS)
