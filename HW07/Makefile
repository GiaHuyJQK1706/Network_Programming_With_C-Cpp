CC = gcc
CFLAGS = -Wall -g

all: server client

server: server.c
	$(CC) $(CFLAGS) -o server server.c -lcrypto

client: client.c
	$(CC) $(CFLAGS) -o client client.c

clean:
	rm -f server client *.o