ALL+=$(APPS)
CLEAN+=$(APPS)

%: %.o
ifdef VERBOSE
	$(CC) $(CFLAGS) $(LDFLAGS) -Wl,--start-group $^ $(LDLIBS) $(LDLIBS_GP) -Wl,--end-group -o $@
else
	@echo "LD   $@"
	@$(CC) $(CFLAGS) $(LDFLAGS) -Wl,--start-group $^ $(LDLIBS) $(LDLIBS_GP) -Wl,--end-group -o $@
endif
