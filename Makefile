COMMANDS = all clean clobber test copy run pty debug-run debug-pty

.PHONY: $(COMMANDS)

$(COMMANDS):
	$(MAKE) -C build $@
