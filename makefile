CC = gcc
CFLAGS = -Wall -pthread

SRC = src/main.c src/player.c src/game.c src/network.c src/server.c
OBJ = $(SRC:.c=.o)

TARGET = server

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -Iinclude -o $@ $^

clean:
	rm -f src/*.o $(TARGET)
