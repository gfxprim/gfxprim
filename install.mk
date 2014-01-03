ifdef DESTDIR
BIN_DIR=$(DESTDIR)/$(bindir)
else
BIN_DIR=$(bindir)
endif

ifdef DESTDIR
MAN_DIR=$(DESTDIR)/$(mandir)/man1
else
MAN_DIR=$(mandir)/man1
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
	for i in $(INSTALL_MAN); do install "$$i" "$(MAN_DIR)"; done
else
	@install -d "$(MAN_DIR)"
	@echo "MKDIR $(MAN_DIR)"
	@for i in $(INSTALL_MAN); do echo "CP    $$i $(MAN_DIR)"; install "$$i" "$(MAN_DIR)"; done
endif
endif
