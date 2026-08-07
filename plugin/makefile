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
CXXFLAGS+= -I$(GCC_PLUGINS_DIR)/include -fPIC -fno-rtti -std=gnu++20 -Wall -Wextra -Wpedantic

# our recipes
.PHONY: all clean

all: bin/uprint.so

bin/uprint.so: $(PLUGIN_SOURCE_FILES)
	mkdir -p bin
	$(HOST_GCC) -Iinclude -shared $(CXXFLAGS) $^ -o $@

install: bin/uprint.so
	sudo cp $(shell pwd)/$^ $(GCC_PLUGINS_DIR)/uprint.so

clean: 
	rm -f bin/*