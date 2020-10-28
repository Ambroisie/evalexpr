CC = gcc
CPPFLAGS = -Isrc/ -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -Wextra -pedantic -Werror -std=c99
VPATH = src/

SRC = \
    src/ast/ast.c \
    src/eval/eval.c \
    src/parse/parse.c \

BIN = evalexpr
OBJ = $(SRC:.c=.o)

.PHONY: all
all: $(BIN)

# Write this one rule instead of using the implicit rules to buid at the root
$(BIN): $(OBJ) src/evalexpr.o

.PHONY: clean
clean:
	$(RM) $(OBJ) # remove object files
	$(RM) $(BIN) # remove main program
