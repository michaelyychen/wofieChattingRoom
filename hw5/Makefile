all: server chat server

client: client.c
	gcc -Wall -Werror -valgrind -o client client.c

chat: chat.c
	gcc -Wall -Werror -valgrind -o chat chat.c

server: server.c
	gcc -Wall -Werror -valgrind -o server server.c

clean:
	rm -f *~ *.o all 
	
