##############
# BUILD TYPE #
##############
BUILD_TYPE ?= debug
ifeq ($(BUILD_TYPE),release)
  FLAGS := -O2 -DNDEBUG
	EXE_NAME := test-release
else ifeq ($(BUILD_TYPE),asan)
  FLAGS := -g -O1 -fsanitize=address -fno-omit-frame-pointer
  EXE_NAME := test-asan
else
  FLAGS := -g -O0
  EXE_NAME := test
endif

##################
# COMPILER FLAGS #
##################
CC := gcc
CFLAGS := -std=c11 $(FLAGS) \
  -Wall -Wextra -Wno-sign-compare -Wno-unused-parameter \
  -Wno-unused-variable -Wno-unused-function \
  -I.
LDFLAGS := -lpthread -lm

#########
# PATHS #
#########
BUILD_DIR := build

SOURCES := test.c
HEADERS := sp.h
TARGET := $(BUILD_DIR)/$(EXE_NAME)

###########
# TARGETS #
###########
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(TARGET): $(SOURCES) $(HEADERS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SOURCES) $(LDFLAGS) -o $(TARGET)

.PHONY: test clean 

test: $(TARGET)
	./$(TARGET)

clean:
	@rm -rf $(BUILD_DIR)
