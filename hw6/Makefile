all: server client chat

client: client.c
	gcc -Wall -Werror -g -o client client.c

chat: chat.c
	gcc -Wall -Werror  -g -o chat chat.c 

server: server.o sfwrite.o
	gcc -Wall -Werror -o server server.o sfwrite.o -pthread -lcrypto -lssl
server.o: server.c 
	gcc -Wall -Werror -c server.c
sfwrite.o: sfwrite.c
	gcc -Wall -Werror -c sfwrite.c

clean:
	rm -f *~ *.o server client chat
	
