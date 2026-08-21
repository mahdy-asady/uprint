PLUGIN_PATH ?= ../../plugin/bin/uprint.so
INC_DIR     ?= ../../include

CC       := gcc
CFLAGS   := -O2 -I$(INC_DIR) -fplugin=$(PLUGIN_PATH) -Wall -Wextra -fanalyzer -Wpedantic
TARGET   := bin/test_app

all: $(TARGET)

$(TARGET): main.c emitter.c
	@mkdir -p bin
	@rm -f uprint.db
	@$(CC) $(CFLAGS) main.c emitter.c third.c -o $(TARGET)

clean:
	rm -rf bin
	rm -f uprint.db *.o
