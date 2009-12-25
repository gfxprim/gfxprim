all:
	$(MAKE) -C engine
	$(MAKE) -C line
	$(MAKE) -C triangle

clean:
	$(MAKE) clean -C engine
	$(MAKE) clean -C line
	$(MAKE) clean -C triangle

benchmark:
	@./run.sh
