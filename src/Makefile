CC = gcc
CFLAGS = -Wall -Wextra -pedantic
LDFLAGS = -lncurses -lpthread

SRC = game.c game.h threads.h globals.h
OBJ = $(SRC:.c=.o)
EXECUTABLE = game

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean

clean:
	rm -f $(OBJ) $(EXECUTABLE)
