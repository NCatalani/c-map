# Main Makefile
RED=\033[0;31m
GREEN=\033[0;32m
NC=\033[0m # No Color

define print
	@echo -e "$(1)$(2)$(NC)"
endef

CLANG_CODE_STYLE='webkit'

all: lib test

run: clean test
	$(call print,$(GREEN),"Running test...")
	@./out/test

lib:
	$(call print,$(GREEN),"Building library...")
	@$(MAKE) -C src

test: lib
	$(call print,$(GREEN),"Building test...")
	@$(MAKE) -C test

.PHONY: clean lib test format

clean:
	$(call print,$(RED),"Cleaning up...")
	@$(MAKE) -C src clean
	@$(MAKE) -C test clean

format:
	$(call print,$(GREEN),"Formatting code...")
	@FILES=$$(find . -type f \( -iname \*.c -o -iname \*.h \)); \
	for FILE in $$FILES; do \
		clang-format --style=$(CLANG_CODE_STYLE) -i "$$FILE"; \
		echo -e "${GREEN}Formatted $$FILE${NC}"; \
	done
