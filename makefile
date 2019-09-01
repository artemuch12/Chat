all: client server
client: client.o
	gcc client.o -o client -lpthread
	rm client.o
client.o: client.c
	gcc -g client.c -c
server: server.o
	gcc server.o -o server -lpthread
	rm server.o
server.o: server.c
	gcc -g server.c -c
