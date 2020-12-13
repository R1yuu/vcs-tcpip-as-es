all: simple_message_client simple_message_server
simple_message_client: simple_message_client.o
	gcc -o simple_message_client simple_message_client.o -lsimple_message_client_commandline_handling
simple_message_client.o: simple_message_client.c
	gcc -c -Wall -Werror -Wextra -Wstrict-prototypes -Wformat=2 -pedantic -fno-common -ftrapv -O3 -g -std=gnu11 simple_message_client.c
simple_message_server: simple_message_server.o
	gcc -o simple_message_server simple_message_server.o -lsimple_message_client_commandline_handling
simple_message_server.o: simple_message_server.c
	gcc -c -Wall -Werror -Wextra -Wstrict-prototypes -Wformat=2 -pedantic -fno-common -ftrapv -O3 -g -std=gnu11 simple_message_server.c
clean:
	rm -f *.o
	rm -f *.out
	rm -f simple_message_client
	rm -f simple_message_server
	rm -f *.html
