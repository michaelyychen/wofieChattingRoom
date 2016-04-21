all: server client chat

client: client.c
	gcc -Wall -Werror -g -o client client.c

chat: chat.c
	gcc -Wall -Werror  -g -o chat chat.c

server: server.c
	gcc -Wall -Werror -pthread -g -o server server.c

clean:
	rm -f *~ *.o server client
	
