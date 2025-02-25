CC = gcc
CPPFLAGS = -Isrc/ -D_POSIX_C_SOURCE=200809L -D_USE_CLIMBING=$(USE_CLIMBING)
CFLAGS = -Wall -Wextra -pedantic -Werror -std=c99
VPATH = src/ tests/
USE_CLIMBING = 1

SRC = \
    src/ast/ast.c \
    src/eval/eval.c \
    src/parse/climbing_parse.c \
    src/parse/recursive_parse.c \

BIN = evalexpr
OBJ = $(SRC:.c=.o)

.PHONY: all
all: $(BIN)

# Write this one rule instead of using the implicit rules to buid at the root
$(BIN): $(OBJ) src/evalexpr.o

check: testsuite
	./testsuite --verbose

TEST_SRC = \
    tests/climbing.c \
    tests/recursive.c \
    tests/testsuite.c \

TEST_OBJ = $(TEST_SRC:.c=.o)

testsuite: LDFLAGS+=-lcriterion -fsanitize=address
testsuite: CFLAGS+=-fsanitize=address
testsuite: $(OBJ) $(TEST_OBJ)

.PHONY: clean
clean:
	$(RM) $(OBJ) # remove object files
	$(RM) $(BIN) # remove main program
