PLUGIN_PATH ?= ../../plugin/bin/uprint.so
INC_DIR     ?= ../../include

CC       := gcc
CFLAGS   := -O2 -I$(INC_DIR) -Wall -Wextra -fanalyzer -Wpedantic
PL_FLAGS := -fplugin=$(PLUGIN_PATH)
TARGET   := bin/test_app

all: $(TARGET)

$(TARGET): main.c
	@mkdir -p bin
	@rm -f uprint.db
	@$(CC) $(CFLAGS) $(PL_FLAGS) $< -o $@

no_uprint: main.c
	@mkdir -p bin
	@rm -f uprint.db
	@$(CC) $(CFLAGS) $< -o $(TARGET)

run:
	@bin/test_app

clean:
	rm -rf bin
	rm -f uprint.db *.o
