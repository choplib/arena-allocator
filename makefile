CC = clang

CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -g
SANFLAGS = -fsanitize=address,undefined -fno-omit-frame-pointer
INCLUDES = -Isrc/arena

ARENA_SRC = src/arena/arena.c
MAIN_SRC = src/main.c
TEST_SRC = src/test/test.c

BIN = bin/out
TEST_BIN = bin/test

.PHONY: all run test clean

all: $(BIN)

$(BIN): $(ARENA_SRC) $(MAIN_SRC) src/arena/arena.h
	@mkdir -p bin
	$(CC) $(CFLAGS) $(INCLUDES) $(ARENA_SRC) $(MAIN_SRC) -o $@

run: $(BIN)
	./$(BIN)

$(TEST_BIN): $(ARENA_SRC) $(TEST_SRC) src/arena/arena.h
	@mkdir -p bin
	$(CC) $(CFLAGS) $(SANFLAGS) $(INCLUDES) $(ARENA_SRC) $(TEST_SRC) -o $@

test: $(TEST_BIN)
	./$(TEST_BIN)

clean:
	rm -rf bin
