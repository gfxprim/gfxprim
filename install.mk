include $(TOPDIR)/libver.mk

ifdef DESTDIR
BIN_DIR=$(DESTDIR)/$(bindir)
MAN_DIR=$(DESTDIR)/$(mandir)/man1
DOC_DIR=$(DESTDIR)/$(docdir)/libgfxprim-$(LIB_VERSION)
PKGCONFIG_DIR=$(DESTDIR)/$(libdir)/pkgconfig/
else
MAN_DIR=$(mandir)/man1
BIN_DIR=$(bindir)
DOC_DIR=$(docdir)/libgfxprim-$(LIB_MAJOR)-$(LIB_MINOR)
PKGCONFIG_DIR=$(libdir)/pkgconfig/
endif

install:
ifdef INSTALL_BIN
ifdef VERBOSE
	install -d "$(BIN_DIR)"
	for i in $(INSTALL_BIN); do install "$$i" "$(BIN_DIR)"; done
else
	@install -d "$(BIN_DIR)"
	@echo "MKDIR $(BIN_DIR)"
	@for i in $(INSTALL_BIN); do echo "CP    $$i $(BIN_DIR)"; install "$$i" "$(BIN_DIR)"; done
endif
endif

ifdef INSTALL_MAN
ifdef VERBOSE
	install -d "$(MAN_DIR)"
	for i in $(INSTALL_MAN); do install -m 644 "$$i" "$(MAN_DIR)"; done
else
	@install -d "$(MAN_DIR)"
	@echo "MKDIR $(MAN_DIR)"
	@for i in $(INSTALL_MAN); do echo "CP    $$i $(MAN_DIR)"; install -m 644 "$$i" "$(MAN_DIR)"; done
endif
endif

ifdef INSTALL_DOC
ifdef VERBOSE
	install -d "$(DOC_DIR)"
	for i in $(INSTALL_DOC); do install -D -m 644 "$$i" "$(DOC_DIR)/$$i"; done
else
	@install -d "$(DOC_DIR)"
	@echo "MKDIR $(DOC_DIR)"
	@for i in $(INSTALL_DOC); do echo "CP    $$i $(DOC_DIR)/$$i"; install -D -m 644 "$$i" "$(DOC_DIR)/$$i"; done
endif
endif

ifdef INSTALL_PKGCONFIG
ifdef VERBOSE
	install -d "$(PKGCONFIG_DIR)"
	for i in $(INSTALL_PKGCONFIG); do install -D -m 644 "$$i" "$(PKGCONFIG_DIR)/$$i"; done
else
	@install -d "$(PKGCONFIG_DIR)"
	@echo "MKDIR $(PKGCONFIG_DIR)"
	@for i in $(INSTALL_PKGCONFIG); do echo "CP    $$i $(PKGCONFIG_DIR)/$$i"; install -D -m 644 "$$i" "$(PKGCONFIG_DIR)/$$i"; done
endif
endif
