CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c11

TARGETS = client server

SOURCES = client.c server.c

OBJECTS = $(SOURCES:.c=.o)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

all: $(TARGETS)

client: client.o
	$(CC) $(CFLAGS) client.o -o client

server: server.o
	$(CC) $(CFLAGS) server.o -o server

clean:
	rm -f $(OBJECTS) $(TARGETS) *.txt

.PHONY: all clean
