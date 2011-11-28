#
# This is makefile rule for generating C sources from python generators
#
ifndef LIBNAME
$(error LIBNAME not defined, fix your library Makefile)
endif

ifndef GENHEADERS
GENHEADERS=
endif

ifndef GENSOURCES
GENSOURCES=
endif

#
# Headers go into include/core/
#
INCLUDE_PREFIX=$(TOPDIR)/include/$(LIBNAME)/
RGENHEADERS=$(addprefix $(INCLUDE_PREFIX),$(GENHEADERS))

#
# Generate genfiles for generated sources
#
CSOURCES+=$(GENSOURCES)

#
# Make the genrated headers actually build
#
all: $(GENHEADERS)

#
# Base common templates location
#
TEMPLATE_DIR=$(TOPDIR)/pylib/templates/

#
# ALL templates and potential generated files (not generated automatically)
# NOTE: Currently unused
# 
ALL_TEMPLATES=$(shell find $(TOPDIR) -name '*.t')
ALL_GENERATED=$(basename $(ALL_TEMPLATES))

#
# And clean them
#
CLEAN+=$(GENSOURCES) $(GENHEADERS)

#
# Generated files depend on python generators and the template
#
$(GENSOURCES) $(GENHEADERS): %: %.t $(PYTHON_FILES)
ifdef VERBOSE
	${PYTHON} ${TOPDIR}/pylib/bin/generate_file.py -t $(TEMPLATE_DIR) "$@.t" "$@"
else
	@echo "GEN  $@"
	@${PYTHON} ${TOPDIR}/pylib/bin/generate_file.py -t $(TEMPLATE_DIR) "$@.t" "$@"
endif

