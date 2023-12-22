# src/Makefile

RED=\033[0;31m
GREEN=\033[0;32m
NC=\033[0m # No Color

define print
	@echo -e "$(1)$(2)$(NC)"
endef

CC=gcc
CFLAGS=-I. -Wno-deprecated-declarations -fPIC
OUT_DIR=../out
LIB_NAME=libmap

SRCS=$(wildcard *.c)
OBJS=$(patsubst %.c,$(OUT_DIR)/%.o,$(SRCS))

all: dirs static dynamic

dirs:
	@mkdir -p $(OUT_DIR)

static: $(OUT_DIR)/$(LIB_NAME).a

dynamic: $(OUT_DIR)/$(LIB_NAME).so

$(OUT_DIR)/$(LIB_NAME).a: $(OBJS)
	$(call print,$(GREEN),"Creating static library...")
	ar rcs $@ $(OBJS)

$(OUT_DIR)/$(LIB_NAME).so: $(OBJS)
	$(call print,$(GREEN),"Creating dynamic library...")
	$(CC) -shared -o $@ $(OBJS)

$(OUT_DIR)/%.o: %.c
	$(call print,$(GREEN),"Compiling $<...")
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean

clean:
	$(call print,$(RED),"Cleaning up...")
	rm -f $(OUT_DIR)/*.o
	rm -rf $(OUT_DIR)/$(LIB_NAME).a
	rm -rf $(OUT_DIR)/$(LIB_NAME).so
