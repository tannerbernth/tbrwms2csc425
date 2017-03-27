// Robert Walters
// Tanner Bernth
// CSC 425

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

int main(int argc, char *argv[]){

	int clientSock, newSocket;
	struct sockaddr_in serverAddr;
	struct sockaddr_storage serverStorage;
	socklen_t addr_size;
	int sport = atoi(argv[1]);

	clientSock = socket(PF_INET, SOCK_STREAM, 0);

	// build the struct for connection to telnet daemon -------------------------
	int daemonSock = socket(PF_INET, SOCK_STREAM, 0);
	struct sockaddr_in telnetAddr;

	memset(&telnetAddr, '\0', sizeof(telnetAddr));
	telnetAddr.sin_family = AF_INET;
	telnetAddr.sin_port = htons(23);
	//telnetAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	if (inet_pton(AF_INET, "127.0.0.1", &telnetAddr.sin_addr) < 1) {
		fprintf(stderr,"Error with telnet daemon\n");
		exit(1);
	}

	// connect to the telnet daemon
	addr_size = sizeof serverAddr;
	if (connect(daemonSock, (struct sockaddr *)  &telnetAddr, sizeof(telnetAddr)) < 0) {
		fprintf(stderr, "Error connecting to telnet daemon\n");
		exit(1);
	}
	//---------------------------------------------------------------------------

	// build struct for server --------------------------------------------------
	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(sport);
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	
	// start listening for client 
	if (bind(clientSock, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0) {
		fprintf(stderr,"Error binding to cproxy socket\n");
		exit(1);
	}
	if(listen(clientSock,5000)!=0){
		printf("Error\n");
	} else {
		printf("Listen on port %d\n",sport);
	}
	addr_size = sizeof(serverStorage);
	newSocket = accept(clientSock, (struct sockaddr *) &serverStorage, &addr_size);
	if (newSocket < 0) {
		fprintf(stderr, "Error: could not connect with cproxy\n");
		exit(1);
	}
 
 	//---------------------------------------------------------------------------

	fd_set readfds;
	struct timeval tv;
	int n,
		telnetBytes,
		cproxyBytes;
	char telnetBuffer[1024],
		 cproxyBuffer[1024];

	while (1) {
		FD_ZERO(&readfds);
		FD_SET(clientSock, &readfds);
		FD_SET(daemonSock, &readfds);
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		if (clientSock > daemonSock) n = clientSock+1;
		else n = daemonSock+1;
		int rv = select(n, &readfds, NULL, NULL, &tv);
		if (rv == -1) {
			fprintf(stderr,"Error: select failed\n");
			exit(1);
		} else if (rv == 0) {
			printf("Timeout occured");
		} else {
			// If receive data from client, print it
			if (FD_ISSET(clientSock, &readfds)) {
				memset(cproxyBuffer, 0, sizeof(cproxyBuffer));
				cproxyBytes = recv(clientSock,cproxyBuffer,sizeof(cproxyBuffer),0);
				if (cproxyBytes > 0) {
					send(clientSock,cproxyBuffer,sizeof(cproxyBuffer),0);
					cproxyBytes = 0;
				}
			}
			// If receive data from telnet, send to client
			if(FD_ISSET(daemonSock,&readfds)) {
				memset(telnetBuffer, 0, sizeof(telnetBuffer));
				telnetBytes = recv(daemonSock, telnetBuffer, sizeof(telnetBuffer), 0);
				if (telnetBytes > 0) {
					send(daemonSock,telnetBuffer,sizeof(telnetBuffer),0);
					telnetBytes = 0;
				}
			}
		}
	}

	return 0;
}