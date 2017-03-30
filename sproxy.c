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
	socklen_t addr_size;
	int sport = atoi(argv[1]);

	// build the struct for connection to telnet daemon -------------------------
	struct sockaddr_in telnetAddr;

	memset(&telnetAddr, '\0', sizeof(telnetAddr));
	telnetAddr.sin_family = AF_INET;
	telnetAddr.sin_port = htons(23);
	//telnetAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	if (inet_pton(AF_INET, "127.0.0.1", &telnetAddr.sin_addr) < 1) {
		fprintf(stderr,"Error with telnet daemon\n");
		exit(1);
	}

	int daemonSock = socket(AF_INET, SOCK_STREAM, 0);
	if (daemonSock < 0) {
		fprintf(stderr, "Error with telnet daemon\n");
		exit(1);
	}

	// connect to the telnet daemon
	if (connect(daemonSock, (struct sockaddr *)  &telnetAddr, sizeof(telnetAddr)) < 0) {
		fprintf(stderr, "Error connecting to telnet daemon\n");
		exit(1);
	}
	//---------------------------------------------------------------------------

	// build struct for server --------------------------------------------------
	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;//inet_addr("127.0.0.1");
	serverAddr.sin_port = htons(sport);

	clientSock = socket(AF_INET, SOCK_STREAM, 0);
	if (clientSock < 0) {
		fprintf(stderr, "Error with cproxy socket\n");
		exit(1);
	}
	
	int yes = 1;
	if (setsockopt(clientSock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
    	fprintf(stderr,"Error resetting socket\n");
    	exit(1);
	}
	// start listening for client 
	if (bind(clientSock, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0) {
		fprintf(stderr,"Error binding to cproxy socket\n");
		exit(1);
	}
	listen(clientSock,5000);
	printf("Listen on port %d\n",sport);
	addr_size = sizeof(serverAddr);
	newSocket = accept(clientSock, (struct sockaddr *) &serverAddr, &addr_size);
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
		FD_SET(newSocket, &readfds);
		FD_SET(daemonSock, &readfds);
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		int maxfd;
		if (newSocket > daemonSock) maxfd = newSocket+1;
		else maxfd = daemonSock+1;
		n = select(maxfd, &readfds, NULL, NULL, &tv);
		if (n == -1) {
			fprintf(stderr,"Error: select failed\n");
			exit(1);
		} else if (n == 0) {
			printf("Timeout occured");
		} else {
			// If receive data from client, print it
			if (FD_ISSET(newSocket, &readfds)) {
				memset(cproxyBuffer, 0, sizeof(cproxyBuffer));
				cproxyBytes = recv(newSocket,cproxyBuffer,sizeof(cproxyBuffer),0);
				if (cproxyBytes > 0) {
					send(daemonSock,cproxyBuffer,sizeof(cproxyBuffer),0);
					cproxyBytes = 0;
				}
			}
			// If receive data from telnet, send to client
			if(FD_ISSET(daemonSock,&readfds)) {
				memset(telnetBuffer, 0, sizeof(telnetBuffer));
				telnetBytes = recv(daemonSock, telnetBuffer, sizeof(telnetBuffer), 0);
				if (telnetBytes > 0) {
					send(newSocket,telnetBuffer,sizeof(telnetBuffer),0);
					telnetBytes = 0;
				}
			}
		}
	}

	return 0;
}