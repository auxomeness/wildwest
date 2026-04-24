CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -pthread
INCLUDES = -Iinclude

# Shared gameplay code is linked into both binaries.
COMMON_SRC = src/player.c src/game.c src/network.c
SERVER_SRC = src/main.c src/server.c
CLIENT_SRC = src/client.c

COMMON_OBJ = $(COMMON_SRC:.c=.o)
SERVER_OBJ = $(SERVER_SRC:.c=.o)
CLIENT_OBJ = $(CLIENT_SRC:.c=.o)

all: server client

server: $(COMMON_OBJ) $(SERVER_OBJ)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^

client: $(COMMON_OBJ) $(CLIENT_OBJ)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^

src/%.o: src/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f src/*.o server client
