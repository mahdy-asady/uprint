PLUGIN_PATH ?= ../../plugin/bin/uprint.so
INC_DIR     ?= ../../include

USE_UPRINT ?= 1

ifeq ($(USE_UPRINT), 1)
    PL_FLAGS := -fplugin=$(PLUGIN_PATH)
else
    PL_FLAGS :=
endif

CC       := gcc
CFLAGS   := -O2 -I$(INC_DIR) $(PL_FLAGS) -Wall -Wextra -fanalyzer -Wpedantic
TARGET   := bin/test_app

all: $(TARGET)

$(TARGET): main.c
	@mkdir -p bin
	@rm -f uprint.db
	@$(CC) $(CFLAGS) $< -o $@

run:
	@bin/test_app

clean:
	rm -rf bin
	rm -f uprint.db *.o
