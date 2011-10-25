ifndef APPS
$(error APPS not defined, fix your library Makefile)
endif

LDFLAGS+=-lpng -ljpeg

all: $(APPS)

CLEAN+=$(APPS)

%: %.o
ifdef VERBOSE
	$(CC) $(CFLAGS) $(LDFLAGS) -Wl,--start-group $(LDLIBS) $^ -Wl,--end-group -o $@
else
	@echo "LD  $@"
	@$(CC) $(CFLAGS) $(LDFLAGS) -Wl,--start-group $(LDLIBS) $^ -Wl,--end-group -o $@
endif
