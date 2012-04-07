LDLIBS+=-lm -ldl

ALL+=$(APPS)
CLEAN+=$(APPS)

%: %.o
ifdef VERBOSE
	$(CC) $(CFLAGS) $(LDFLAGS) -Wl,--start-group $(LDLIBS) $^ -Wl,--end-group -o $@
else
	@echo "LD  $@"
	@$(CC) $(CFLAGS) $(LDFLAGS) -Wl,--start-group $(LDLIBS) $^ -Wl,--end-group -o $@
endif
