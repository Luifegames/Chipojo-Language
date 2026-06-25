CC = gcc
CFLAGS = -Wall -Ichipojo-interpreter/include -g
OBJ_DIR = obj
BIN = chipojo

SRCS = $(wildcard chipojo-interpreter/src/*.c)
OBJS = $(patsubst chipojo-interpreter/src/%.c,$(OBJ_DIR)/%.o,$(SRCS))

.PHONY: all clean test

all: $(BIN)

$(OBJ_DIR)/%.o: chipojo-interpreter/src/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN): $(OBJS)
	$(CC) $^ -o $@

test: $(BIN)
	@echo "Running tests..."
	@for t in test/*.chp; do \
		echo "[test] $$t"; \
		./$(BIN) "$$t" 2>&1 || echo "[FAIL] $$t"; \
	done

clean:
	rm -rf $(OBJ_DIR) $(BIN)
