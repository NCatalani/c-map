# Main Makefile
RED=\033[0;31m
GREEN=\033[0;32m
NC=\033[0m # No Color

define print
	@echo -e "$(1)$(2)$(NC)"
endef

all: lib test

run: clean test
	$(call print,$(GREEN),"Running test...")
	@./out/test2

lib:
	$(call print,$(GREEN),"Building library...")
	@$(MAKE) -C src

test: lib
	$(call print,$(GREEN),"Building test...")
	@$(MAKE) -C test

.PHONY: clean lib test

clean:
	$(call print,$(RED),"Cleaning up...")
	@$(MAKE) -C src clean
	@$(MAKE) -C test clean
