#
# This is makefile rule for generating C sources from python templates
#
ifndef GENHEADERS
GENHEADERS=
endif

ifndef GENSOURCES
GENSOURCES=
endif

#
# We add these to CSOURCES which is handled in post.mk
#
CSOURCES+=$(GENSOURCES)

#
# Make the genrated headers actually build
#
ALL+=$(GENHEADERS) $(GENSOURCES)

#
# Base common templates location
#
TEMPLATEDIR=$(TOPDIR)/gen/include/
CCT=$(TOPDIR)/gen/bin/cct.py

#
# And clean them
#
CLEAN+=$(GENSOURCES) $(GENHEADERS)

#
# Some base dependencies
#
$(GENSOURCES): $(TEMPLATEDIR)/source.t $(TEMPLATEDIR)/license.t
$(GENHEADERS): $(TEMPLATEDIR)/header.t $(TEMPLATEDIR)/license.t

#
# Generated files depend on python generators and the template
#
$(GENSOURCES) $(GENHEADERS): %: %.t
ifdef VERBOSE
	PYTHONPATH=$(TEMPLATEDIR) ${CCT} -I $(TEMPLATEDIR) "$@.t"
else
	@echo "CCT  $@"
	@PYTHONPATH=$(TEMPLATEDIR) ${CCT} -I $(TEMPLATEDIR) "$@.t"
endif
