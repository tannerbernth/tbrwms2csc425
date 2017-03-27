# Robert Walters
# Tanner Bernth
# CSC 425

make:
	gcc -Wall -o client client.c
	gcc -Wall -o server server.c
	gcc -Wall -o cproxy cproxy.c
	gcc -Wall -o sproxy sproxy.c
