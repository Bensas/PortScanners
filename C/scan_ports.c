#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#define DEFAULT_START_PORT 0
#define DEFAULT_END_PORT 65535
#define true 1
#define false 0


int strIsPositiveNumber(char* str){
	int i = 0;
	while (str[i] != 0)
		if (!isdigit(str[i++]))
			return false;
	return true;
}

void testStrIsPositiveNumber() {
	printf("testStrIsPositiveNumber()\n");
	char* positiveNum = "1920";
	char* negativeNum = "-3232";
	char* nonNumber = "67fs 0";
	assert(strIsPositiveNumber(positiveNum) == true);
	assert(strIsPositiveNumber(negativeNum) == false);
	assert(strIsPositiveNumber(nonNumber) == false);
}

struct in_addr resolveHostnameToAddr(char* hostname) {

    struct addrinfo* info;

    int err1 = getaddrinfo(hostname, NULL, NULL, &info);

    if (err1 != 0) {
        printf("Failed to resolve provided hostname. Exiting.\n");
        exit(1);
    }

    return (struct in_addr)((struct sockaddr_in *)info->ai_addr)->sin_addr;

    // struct sockaddr_in* addr = (struct sockaddr_in *)result->ai_addr; 
    // printf("%s\n", inet_ntoa((struct in_addr)addr->sin_addr));
}

void scanPorts(char* hostname, int startPort, int endPort) {
	struct sockaddr_in* sockAddr;
	int sock;
	sockAddr = malloc(sizeof(struct sockaddr_in));
	sockAddr->sin_family = AF_INET;
	sockAddr->sin_addr = resolveHostnameToAddr(hostname);

	printf("Scanning...\n");

	for (int port = startPort; port <= endPort; port++) {
		
		sockAddr->sin_port = htons(port);

		sock = socket(AF_INET , SOCK_STREAM , 0);
		
		if(sock < 0) {
			perror("Socket\n");
			exit(1);
		}

		int err = connect(sock, (struct sockaddr *)sockAddr, sizeof(struct sockaddr_in));
		
		if( err >= 0 ) {
			printf("%-5d open\n",  port);
		} else {
			printf("%-5d closed\n",  port);
		}

		close(sock);
	}
}


void runTests() {
	printf("- - - Running Tests - - -\n");
	testStrIsPositiveNumber();
	printf("- - - Success! - - -\n");
}

int main(int argc, char** argv) {

	if (argc < 2) {
        printf("Please specify a target IP.\nUsage: scan_ports [ip_add] [start_port] [end_port]\n");
        exit(1);
    }

    if (!strcmp(argv[1], "test")) {
    	runTests();
    }

    char* ipAddress = argv[1];
    int startPort = DEFAULT_START_PORT;
    int endPort = DEFAULT_END_PORT;

    if (argc < 4) {
        printf("No start or end port specified, scanning from %d to %d", startPort, endPort);
    } else {
    	if (!strIsPositiveNumber(argv[2]) || !strIsPositiveNumber(argv[3])) {
    		printf("Please enter valid port numbers. Exiting.\n");
    		exit(1);
    	}
        startPort = atoi(argv[2]);
        endPort = atoi(argv[3]);
    }

    scanPorts(ipAddress, startPort, endPort);
}
