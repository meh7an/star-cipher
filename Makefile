CC = gcc
CFLAGS = -Wall -Wextra -O2
TARGET = star_cipher

SRC_DIR = src
BIN_DIR = bin

SRCS = $(wildcard $(SRC_DIR)/*.c)

all: $(BIN_DIR)/$(TARGET)

$(BIN_DIR)/$(TARGET): $(SRCS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(BIN_DIR)/$(TARGET)

.PHONY: all clean
