CC = gcc
CFLAGS = -Wall -Iinclude -g
OBJ_DIR = obj
BIN = chipojo

SRCS = $(wildcard src/*.c)
OBJS = $(patsubst src/%.c,$(OBJ_DIR)/%.o,$(SRCS))

.PHONY: all clean chpm test

all: $(BIN)

$(OBJ_DIR)/%.o: src/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN): $(OBJS)
	$(CC) $^ -o $@

chpm:
	$(MAKE) -C chpm

test: $(BIN)
	@echo "Running tests..."
	@for t in test/*.chp; do \
		echo "[test] $$t"; \
		./$(BIN) "$$t" 2>&1 || echo "[FAIL] $$t"; \
	done

clean:
	rm -rf $(OBJ_DIR) $(BIN)
	$(MAKE) -C chpm clean
