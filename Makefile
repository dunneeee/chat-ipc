CC = gcc
CFLAGS = -Wall -g -Iincludes
LDFLAGS = -lpthread

all: server client

server: src/server.c src/utils.c
	$(CC) $(CFLAGS) -o bin/server src/server.c src/utils.c $(LDFLAGS)

client: src/client.c src/utils.c
	$(CC) $(CFLAGS) -o bin/client src/client.c src/utils.c $(LDFLAGS)

clean:
	rm -f bin/server bin/client