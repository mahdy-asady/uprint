DEBUG ?= 0

# The gcc used to compile our plugin (consider that is written in C++!)
HOST_GCC = g++

# The gcc that will support our plugin 
TARGET_GCC = gcc

# The name of the file we want to compile 
PLUGIN_SOURCE_FILES = src/main.cpp \
					  src/pass_uprint_lower.cpp \
					  src/database.cpp

# Plugins directory of GCC
GCC_PLUGINS_DIR = $(shell $(TARGET_GCC) -print-file-name=plugin)

# GCC CPP flags (we need position independent-code and run-time type info support disabled)
CXXFLAGS+= -isystem$(GCC_PLUGINS_DIR)/include -fPIC -fno-rtti -std=gnu++20 -Wall -Wextra -fanalyzer -Wpedantic

# Check if DEBUG flag is set
ifeq ($(DEBUG), 1)
    CXXFLAGS += -DENABLE_DEBUG -g -O0
else
    CXXFLAGS += -O2
endif

# our recipes
.PHONY: all clean

all: cppcheck

cppcheck: bin/uprint.so
	@echo
	@echo Start Static Analysis...
	@cppcheck -q --enable=all --std=c++17 --language=c++ \
			  -Iinclude -isystem$(GCC_PLUGINS_DIR)/include \
			  --suppress=missingIncludeSystem \
			  --suppress=unusedFunction:src/main.cpp \
			  --suppress=unusedFunction:src/pass_uprint_lower.cpp \
			  --error-exitcode=1 src
	@echo Static Analysis Done!
	@echo

bin/uprint.so: $(PLUGIN_SOURCE_FILES)
	mkdir -p bin
	$(HOST_GCC) -Iinclude -shared $(CXXFLAGS) $^ -o $@

install: bin/uprint.so
	sudo cp $(shell pwd)/$^ $(GCC_PLUGINS_DIR)/uprint.so

clean: 
	rm -f bin/*