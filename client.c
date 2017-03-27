// Robert Walters
// Tanner Bernth
// CSC 425

#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]){
	char *sip = argv[1];
	int sport = atoi(argv[2]);
	size_t size;
	char *buffer = malloc(sizeof(char)*1024);
	struct sockaddr_in serverAddr;
	socklen_t addr_size;

	int sock = socket (PF_INET, SOCK_STREAM, 0);

	// build the struct
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(sport);
	serverAddr.sin_addr.s_addr = inet_addr(sip);
	memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

	// connect to the server
	addr_size = sizeof serverAddr;
	connect(sock, (struct sockaddr *)  &serverAddr, addr_size);

	// send message to server
	while(getline(&buffer, &size, stdin) > 0){
		send(sock,buffer,(int)strlen(buffer),0);
		//free(buffer);
		memset(buffer, '\0', 1024);
	}

	return 0;

}
