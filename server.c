// Robert Walters
// Tanner Bernth
// CSC 425

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

int main(int argc, char *argv[]){

	int welcomeSocket, newSocket;
	char buffer[1024];
	struct sockaddr_in serverAddr;
	struct sockaddr_storage serverStorage;
	socklen_t addr_size;
	int sport = atoi(argv[1]);

	welcomeSocket = socket(PF_INET, SOCK_STREAM, 0);

	// build struct
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(sport);
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

	// start listening
	bind(welcomeSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

	if(listen(welcomeSocket,5)!=0){
		printf("Error\n");
	}

	addr_size = sizeof(serverStorage);
	newSocket = accept(welcomeSocket, (struct sockaddr *) &serverStorage, &addr_size);
	memset(buffer, '\0', 1024);
	while(recv(newSocket, buffer, 1024, 0) > 0){
		printf("%d\n", (int)strlen(buffer)-1);
		printf("%s", buffer);
		fflush(stdin);
		memset(buffer, '\0', 1024);
	}

	return 0;
}
