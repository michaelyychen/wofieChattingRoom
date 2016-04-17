all: server client

client: client.c
	gcc -Wall -Werror  -o client client.c

chat: chat.c
	gcc -Wall -Werror  -o chat chat.c

server: server.c
	gcc -Wall -Werror -g -o server server.c

clean:
	rm -f *~ *.o server client
	
