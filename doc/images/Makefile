SUBDIRS=blur median

TARGETS=$(addsuffix /images.txt,$(SUBDIRS))
IMGS=$(addsuffix /*.png,$(SUBDIRS))

all: $(TARGETS)

$(TARGETS):
	@./regen.py

clean:
	rm -f $(TARGETS) $(IMGS)
