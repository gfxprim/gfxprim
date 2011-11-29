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
# Generate genfiles for generated sources
#
CSOURCES+=$(GENSOURCES)

#
# Make the genrated headers actually build
#
all: $(GENHEADERS) $(GENSOURCES)

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
# Add templates as dependencies
#
PYTHON_FILES=$(shell find $(TEMPLATE_DIR) -name '*.t')
$(GENSOURCES) $(GENHEADERS): $(PYTHON_FILES)

#
# Generated files depend on python generators and the template
#
$(GENSOURCES) $(GENHEADERS): %: %.t
ifdef VERBOSE
	${PYTHON} ${TOPDIR}/pylib/bin/generate_file.py -t $(TEMPLATE_DIR) "$@.t" "$@"
else
	@echo "GEN  $@"
	@${PYTHON} ${TOPDIR}/pylib/bin/generate_file.py -t $(TEMPLATE_DIR) "$@.t" "$@"
endif

