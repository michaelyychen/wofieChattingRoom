all: server client chat tool

client: client.o sfwrite.o
	gcc -Wall -Werror -g -o client client.o sfwrite.o -pthread

chat: chat.c
	gcc -Wall -Werror  -g -o chat chat.c sfwrite.o -pthread

tool: tool.c
	gcc -Wall -Werror  -g -o tool tool.c 


server: server.o sfwrite.o
	gcc -Wall -Werror -g -o server server.o sfwrite.o -pthread -lcrypto -lssl

client.o: client.c 
	gcc -Wall -Werror -g -c client.c

server.o: server.c 
	gcc -Wall -Werror -g -c server.c

sfwrite.o: sfwrite.c
	gcc -Wall -Werror -c sfwrite.c

clean:
	rm -f *~ *.o server client chat tool
	
