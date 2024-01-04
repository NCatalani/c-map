# src/Makefile

RED=\033[0;31m
GREEN=\033[0;32m
NC=\033[0m # No Color

define print
	@echo -e "$(1)$(2)$(NC)"
endef

CC=gcc
CFLAGS=-I../include/ -fPIC \
	-Wno-deprecated-declarations \
	-Wall \
	-Wextra \
	-Wundef \
	-Wpointer-arith \
	-Wshadow \
	-Wstrict-prototypes \
	-Wunreachable-code

LIBS=-lssl -lcrypto
ifeq ($(DEBUG), 1)
	CFLAGS+=-g
else
endif
OUT_DIR=../out
LIB_NAME=libcmap

SRCS=$(wildcard *.c)
OBJS=$(patsubst %.c,$(OUT_DIR)/%.o,$(SRCS))

all: static dynamic

static: $(OUT_DIR)/$(LIB_NAME).a

dynamic: $(OUT_DIR)/$(LIB_NAME).so

$(OUT_DIR)/$(LIB_NAME).a: $(OBJS)
	$(call print,$(GREEN),"Creating static library...")
	ar rcs $@ $(OBJS)

$(OUT_DIR)/$(LIB_NAME).so: $(OBJS)
	$(call print,$(GREEN),"Creating dynamic library...")
	$(CC) -shared -o $@ $(OBJS) $(LIBS)

$(OUT_DIR)/%.o: %.c
	$(call print,$(GREEN),"Compiling $<...")
	$(CC) $(CFLAGS) -c $< -o $@ $(LIBS)

.PHONY: clean

clean:
	$(call print,$(RED),"Cleaning up...")
	rm -f $(OUT_DIR)/*.o
	rm -rf $(OUT_DIR)/$(LIB_NAME).a
	rm -rf $(OUT_DIR)/$(LIB_NAME).so
