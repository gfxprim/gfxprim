#
# Install python libraries into right place
#
# Note that LIBNAME is be empty for root gfxprim python package
#
ifdef SWIG
ifdef PYTHON_CONFIG

PY_INSTALL_PREFIX=$(libdir)/python$(PYTHON_VER)/gfxprim/$(LIBNAME)

ifdef DESTDIR
PY_INSTALL_PREFIX:=$(DESTDIR)$(PY_INSTALL_PREFIX)
endif

install: $(INSTALL_FILES)
ifdef VERBOSE
	install -d "$(PY_INSTALL_PREFIX)"
	for i in $(INSTALL_FILES); do install "$$i" "$(PY_INSTALL_PREFIX)"; done
else
	@echo "DEST $(PY_INSTALL_PREFIX)"
	@install -d "$(PY_INSTALL_PREFIX)"
	@for i in $(INSTALL_FILES); do echo "      -> $$i"; install "$$i" "$(PY_INSTALL_PREFIX)"; done
endif

endif
endif
