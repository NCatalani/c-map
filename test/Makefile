# test/Makefile

RED=\033[0;31m
GREEN=\033[0;32m
NC=\033[0m # No Color

define print
	@echo -e "$(1)$(2)$(NC)"
endef

CC=gcc
CFLAGS=-I../include -g
LIBS=-lssl -lcrypto
OUT_DIR=../out
LIB_PATH=$(OUT_DIR)/libcmap.a

#all: $(OUT_DIR)/test $(OUT_DIR)/test2
all: $(OUT_DIR)/test2

#$(OUT_DIR)/test: $(OUT_DIR)/test.o
#	$(call print,$(GREEN),"Linking test program statically...")
#	$(CC) $(CFLAGS) -o $@ $(OUT_DIR)/test.o $(LIB_PATH) $(LIBS)

$(OUT_DIR)/test2: $(OUT_DIR)/test2.o
	$(call print,$(GREEN),"Linking test program statically...")
	$(CC) $(CFLAGS) -o $@ $(OUT_DIR)/test2.o $(LIB_PATH) $(LIBS)

#$(OUT_DIR)/test.o: test.c
#	$(call print,$(GREEN),"Compiling test.c...")
#	$(CC) $(CFLAGS) -c test.c -o $@

$(OUT_DIR)/test2.o: test2.c
	$(call print,$(GREEN),"Compiling test.c...")
	$(CC) $(CFLAGS) -c test2.c -o $@

.PHONY: clean

clean:
	$(call print,$(RED),"Cleaning up test...")
	rm -f $(OUT_DIR)/test.o
#	rm -f $(OUT_DIR)/test
	rm -f $(OUT_DIR)/test2
