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
# And clean them
#
CLEAN+=$(GENSOURCES) $(GENHEADERS)

#
# Some base dependencies
#
$(GENSOURCES): $(TEMPLATE_DIR)/base.c.t $(TEMPLATE_DIR)/common.c.t
$(GENHEADERS): $(TEMPLATE_DIR)/base.h.t

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

