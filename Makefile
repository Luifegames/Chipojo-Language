CC = gcc

CFLAGS = -Wall -Iinclude -g

LIBS = -lraylib -lopengl32 -lgdi32 -lwinmm

SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj

BIN = lizard.exe

SRCS = $(wildcard $(SRC_DIR)/*.c)

OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))

all: $(BIN)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN): $(OBJS)
	$(CC) $^ -o $@ $(LIBS)

clean:
	rm -rf $(OBJ_DIR) $(BIN)

.PHONY: all clean