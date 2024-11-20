CC = gcc
CFLAGS = -I./includes
DEPS = includes/chat.h

all: server client

server: src/server.c $(DEPS)
	$(CC) -o bin/server src/server.c $(CFLAGS)

client: src/client.c $(DEPS)
	$(CC) -o bin/client src/client.c $(CFLAGS)

run-server:
	./bin/server

run-client:
	./bin/client
clean:
	rm -f bin/server bin/client