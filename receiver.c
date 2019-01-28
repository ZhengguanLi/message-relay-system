#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>

int checkSum(char buffer[]){
	// e.g. "abcd$6"
	int len = strlen(buffer); // length of buffer
	int tempLen = 0; // message length

	// get the string represented as the length of buffer appended at buffer after $ sign
	char str[25] = "";
	for(int i = 0;i < len;i++){
		if(buffer[i] == '$'){
			for(int j = i + 1;j < len; j++){
				str[j-i-1] = buffer[j];
			}
			break;
		}
	}

	tempLen = atoi(str); // length extracted from sender
	if(tempLen == len){
		printf("Message complete!\n");	
		return 0; 
	}else{
		printf("Message corrupted!");	
		return -1;
	}
}

void getMessage(char buffer[]){
	char message[25] = "";
	int len = strlen(buffer);
	for(int i = 0; i < len; i++){
		if(buffer[i] == '$') break;
		message[i] = buffer[i];
	}
	printf("message: %s\n", message);
}

int main(int argc, char *argv[]) {
	int simpleSocket = 0;
	int simplePort = 0;
	int returnStatus = 0;
	char buffer[256] = "";
	char buffer2[256] = "";
	struct sockaddr_in simpleServer;
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <port>\n", argv[0]);
		exit(1);
	}

   	simpleSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(simpleSocket == -1){
		fprintf(stderr, "Could not create a socket!\n");
		exit(1);
	}else{
		fprintf(stderr, "Socket created!\n");
   	}
    
	// set up the address structure
	// use INADDR_ANY to bind to all local addresses
	simplePort = atoi(argv[1]); // port number
	bzero(&simpleServer, sizeof(simpleServer));
	simpleServer.sin_family = AF_INET;
	simpleServer.sin_addr.s_addr = htonl(INADDR_ANY);
	simpleServer.sin_port = htons(simplePort);
	
	// bind to the address and port with our socket
	returnStatus = bind(simpleSocket, (struct sockaddr *)&simpleServer, sizeof(simpleServer));
	if (returnStatus == 0) {
		fprintf(stderr, "Bind completed!\n");
	}else {
		fprintf(stderr, "Could not bind to address!\n");
		close(simpleSocket);
		exit(1);
	}
	printf("--------------------------------------\n");

	// listen on the socket for connections
	returnStatus = listen(simpleSocket, 30);
	if (returnStatus == -1) {
		fprintf(stderr, "Cannot listen on socket!\n");
		close(simpleSocket);
		exit(1);
  	}
	
   	while(1){
		int bytes_count = 0;
		char buffer[256] = "";
		char massage[256] = "";
		int cksum = 0;
		struct sockaddr_in clientName = {0};
		int simpleChildSocket = 0;
		int clientNameLength = sizeof(clientName);

		// wait here
		simpleChildSocket = accept(simpleSocket,(struct sockaddr *)&clientName,&clientNameLength);
		if (simpleChildSocket == -1) {
		    fprintf(stderr, "Cannot accept connections!\n");
		    close(simpleSocket);
		    exit(1);
		}

		while(1){
			bzero(buffer, 256);
			bytes_count = recv(simpleChildSocket, buffer, 256, 0);
			
			// if receive message: 'x' or 'q', close socket
			if(strcmp(buffer, "x") == 0 ||strcmp(buffer, "q") == 0){
				close(simpleChildSocket);
				break;
			}else{
				getMessage(buffer);
				cksum = checkSum(buffer);
				bzero(buffer, 256);
				if(cksum == 0){
					strcpy(buffer, "Message Complete!");
				}else {
					strcpy(buffer, "Message Corrupted!");
				}	
				bytes_count = send(simpleChildSocket, buffer, strlen(buffer), 0);
			}
		}
		break;
	}

	// close socket
    return 0;
}
