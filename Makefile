OUT_DIR	= output
SRC = ./src
LINK_TARGET = chip_8.out
SRC_FILES = $(shell find $(SRC) -name '*.c')
OBJS = $(patsubst $(SRC)/%.c, $(OUT_DIR)/%.o, $(SRC_FILES))
REBUILDABLES = $(OBJS) $(LINK_TARGET)
CC_FLAGS = -std=gnu17 -Wall -Werror -Wextra -Wpedantic -Wno-shadow -g -O0

CC = gcc
OUTPUT_FOLDERS = $(addprefix $(OUT_DIR)/, $(notdir $(patsubst $(SRC), , $(shell find $(SRC) -maxdepth 5 -type d))))

all: $(LINK_TARGET)
	@echo "compilation success ✅"

$(LINK_TARGET): $(OBJS)
	$(CC) $(CC_FLAGS) -o $@ $^

$(OUT_DIR)/%.o: $(SRC)/%.c
	$(CC) $(CC_FLAGS) -o $@ -c $<

clean:
	rm -rf $(OUT_DIR)/*
	rm $(LINK_TARGET)
	@echo "clean done ✨"

init:
	mkdir -p $(OUT_DIR) $(OUTPUT_FOLDERS)
	@$(MAKE)

run:
	./$(LINK_TARGET)
