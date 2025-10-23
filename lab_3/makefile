all: server child_1 child_2

server: server.c
	gcc server.c -g -Wall -o server

child_1: child_1.c
	gcc child_1.c -g -Wall -o child_1

child_2: child_2.c
	gcc child_2.c -g -Wall -o child_2

clean:
	rm -rf child_1 child_2 server