CC     = gcc
CFLAGS = -Wall -Wextra

all: client server_fork server_select

client: client.o common.o
	$(CC) $(CFLAGS) -o $@ client.o common.o

server_fork: server_fork.o common.o
	$(CC) $(CFLAGS) -o $@ server_fork.o common.o

server_select: server_select.o common.o
	$(CC) $(CFLAGS) -o $@ server_select.o common.o

%.o: %.c common.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o client server_fork server_select