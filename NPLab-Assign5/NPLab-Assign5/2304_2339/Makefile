CC = gcc
CFLAGS = -Wall -Wextra -g

all: 2304_2339 server

2304_2339: client.o parser.o
	$(CC) $(CFLAGS) -o 2304_2339 client.o parser.o

server: server.o student.o
	$(CC) $(CFLAGS) -o server server.o student.o

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o 2304_2339 server

