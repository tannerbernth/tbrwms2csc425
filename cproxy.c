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
	struct sockaddr_storage telnetStorage;
	socklen_t addr_size;
	socklen_t addr_size2;

	int telnetSock = socket (PF_INET, SOCK_STREAM, 0);

	// build the struct for the server ------------------------------------------
	int serverSock = socket (PF_INET, SOCK_STREAM, 0);

	memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(sport);
	serverAddr.sin_addr.s_addr = inet_addr(sip);

	// connect to the server
	addr_size = sizeof serverAddr;
	if (connect(serverSock, (struct sockaddr *)  &serverAddr, addr_size) < 0) {
		fprintf(stderr,"Error connecting to sproxy\n");
		exit(1);
	}
	printf("Connected on %s:%d\n",sip,sport);
	//---------------------------------------------------------------------------

	//build the struct for client to listen to telnet ---------------------------
	telnetAddr.sin_family = AF_INET;
	telnetAddr.sin_port = htons(tnport);
	telnetAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	memset(telnetAddr.sin_zero, '\0', sizeof telnetAddr.sin_zero);

	// start listening
	bind(telnetSock, (struct sockaddr *) &telnetAddr, sizeof(telnetAddr));

	if(listen(telnetSock,5)!=0){
		printf("Error\n");
	} else {
		printf("Listen on port %d\n",tnport);
	}
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
		FD_SET(serverSock, &readfds);
		FD_SET(telnetSock, &readfds);
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		if (serverSock > telnetSock) n = serverSock+1;
		else n = telnetSock+1;
		int rv = select(n, &readfds, NULL, NULL, &tv);
		if (rv == -1) {
			fprintf(stderr,"Error: select failed\n");
			exit(1);
		} else if (rv == 0) {
			printf("Timeout occured\n");
		} else {
			// If receive data from telnet, send to server
			if(FD_ISSET(telnetSock,&readfds)) {
				memset(telnetBuffer, 0, sizeof(telnetBuffer));
				telnetBytes = recv(telnetSock,telnetBuffer,sizeof(telnetBuffer), 0);
				printf("telnetBytes %d telnetBuffer %d\n",telnetBytes, telnetBuffer);
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
					send(telnetSock,telnetBuffer,sizeof(telnetBuffer),0);
					sproxyBytes = 0;
				}
			}
		}
	}

	return 0;

}

