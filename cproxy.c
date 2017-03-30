#include <stdio.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]){
	char *sip = argv[2];
	int tnport = atoi(argv[1]),
		sport = atoi(argv[3]),
		newSocket;
	struct sockaddr_in serverAddr;
	struct sockaddr_in telnetAddr;
	socklen_t addr_size;
	socklen_t addr_size2;

	// build the struct for the server ------------------------------------------
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(sport);

	if (inet_pton(AF_INET, sip, &serverAddr.sin_addr) < 1) {
		fprintf(stderr, "Error with sproxy IP address\n");
		exit(1);
	}

	int serverSock = socket (AF_INET, SOCK_STREAM, 0);
	if (serverSock < 0) {
		fprintf(stderr, "Error creating server socket \n");
		exit(1);
	}

	// connect to the server
	if (connect(serverSock, (struct sockaddr *)  &serverAddr, sizeof(serverAddr)) < 0) {
		fprintf(stderr,"Error connecting to sproxy\n");
		exit(1);
	}
	printf("Connected on %s:%d\n",sip,sport);
	//---------------------------------------------------------------------------

	//build the struct for client to listen to telnet ---------------------------
	memset(&telnetAddr, 0, sizeof(telnetAddr));
	telnetAddr.sin_family = AF_INET;
	telnetAddr.sin_addr.s_addr = INADDR_ANY;
	telnetAddr.sin_port = htons(tnport);

	int telnetSock = socket (AF_INET, SOCK_STREAM, 0);
	if (telnetSock < 0) {
		fprintf(stderr,"Error with telnet socket\n");
		exit(1);
	}

	int yes = 1;
	if (setsockopt(telnetSock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
		fprintf(stderr,"Error resetting socket\n");
    	exit(1);
	}
	// start listening
	if (bind(telnetSock, (struct sockaddr *) &telnetAddr, sizeof(telnetAddr)) < 0) {
		fprintf(stderr, "Error binding telnet socket\n");
		exit(1);
	}

	listen(telnetSock,100);
	printf("Listen on port %d\n",tnport);

	addr_size2 = sizeof(telnetAddr);
	newSocket = accept(telnetSock, (struct sockaddr *) &telnetAddr, &addr_size2);
	if (newSocket < 0) {
		fprintf(stderr,"Error telnet session failed accept\n");
		exit(1);
	}
	//---------------------------------------------------------------------------

	fd_set readfds;
	struct timeval tv;
	int n,
		telnetBytes,
		sproxyBytes;
	char telnetBuffer[1024],
		 sproxyBuffer[1024];

	while (1) {
		FD_ZERO(&readfds);
		FD_SET(newSocket, &readfds);
		FD_SET(serverSock, &readfds);
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		int maxfd;
		if (newSocket > serverSock) maxfd = newSocket+1;
		else maxfd = serverSock+1;
		n = select(maxfd, &readfds, NULL, NULL, &tv);
		if (n == -1) {
			fprintf(stderr,"Error: select failed\n");
			exit(1);
		} else if (n == 0) {
			printf("Timeout occured\n");
		} else {
			// If receive data from telnet, send to server
			if(FD_ISSET(newSocket,&readfds)) {
				memset(telnetBuffer, 0, sizeof(telnetBuffer));
				telnetBytes = recv(newSocket,telnetBuffer,sizeof(telnetBuffer), 0);
				if (telnetBytes > 0) {
					send(serverSock,telnetBuffer,telnetBytes,0);
					telnetBytes = 0;
				}
			}
			// If receive data from server, print it
			if (FD_ISSET(serverSock, &readfds)) {
				memset(sproxyBuffer, 0, sizeof(sproxyBuffer));
				sproxyBytes =recv(serverSock,sproxyBuffer,sizeof(sproxyBuffer),0);
				
				if (sproxyBytes > 0) {
					send(newSocket,sproxyBuffer,sizeof(sproxyBuffer),0);
					sproxyBytes = 0;
				}
			}
		}
	}

	return 0;

}

