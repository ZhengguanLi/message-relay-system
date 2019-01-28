#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/wait.h>
#include <signal.h>

int main(int argc, char *argv[]) {
	int simpleSocket = 0;
	int simplePort = 0;
	int returnStatus = 0;
	char buffer[256] = "";
	char buffer2[256] = ""; 
	char acknowledgement[256] = "";
	char verification[256] = "";
    int bytes_count = 0;

	struct sockaddr_in simpleServer;
	if (3 != argc) {
	fprintf(stderr, "Usage: %s <server> <port>\n", argv[0]);
	   exit(1);    
	}

	// create a streaming socket
	simpleSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(simpleSocket == -1) {
		fprintf(stderr, "Could not create a socket!\n");
		exit(1);
	}else {
		fprintf(stderr, "Socket created!\n");
	}  

	// set up the address structure
	// use the IP address argument for the server address
	simplePort = atoi(argv[2]); // port number
	bzero(&simpleServer, sizeof(simpleServer));
	simpleServer.sin_family = AF_INET;
	inet_addr(argv[1], &simpleServer.sin_addr.s_addr);
	simpleServer.sin_port = htons(simplePort);
	
	// connect to the address and port with our socket
	returnStatus = connect(simpleSocket,(struct sockaddr *)&simpleServer,sizeof(simpleServer));
	if (returnStatus == 0) {
		fprintf(stderr, "Connect successful!\n");
	}else{
		fprintf(stderr, "Could not connect to address!\n");
		close(simpleSocket);
		exit(1);
	}
	printf("--------------------------------------\n");

	while(1){
		// read username
		printf("Please enter the username: ");
		scanf("%s", buffer); 

		bytes_count = send(simpleSocket, buffer, strlen(buffer), 0);
		bytes_count = recv(simpleSocket, acknowledgement, 256, 0);	
		if(strcmp(acknowledgement, "Acknowledged!") == 0) {
			printf("%s\n",acknowledgement);
		}

		// read password 
		printf("Please enter the password: ");
		scanf("%s", buffer);
		bytes_count = send(simpleSocket, buffer, strlen(buffer), 0);

		// receive verification message
		bytes_count = recv(simpleSocket, buffer, 256, 0);
		if(strcmp(buffer,"Verified!") == 0){
			printf("%s\n", buffer);
			break;
		}else{
			printf("%s\n", buffer);
			printf("--------------------------------------\n");
		}
	}
	
	while(1){
		printf("Please enter the receiver name: ");
		scanf("%s", buffer);

		// send receiver name
		bytes_count = send(simpleSocket, buffer, strlen(buffer), 0);
		
		// send receiver port number
		bytes_count = recv(simpleSocket, acknowledgement, 256, 0);
		if(strcmp(acknowledgement, "Acknowledged!") == 0) {
			// printf("%s\n",acknowledgement);
		}

		// read the port number of receiver
		printf("Please enter the receiver port number: ");
		scanf("%s", buffer2);
		bytes_count = send(simpleSocket, buffer2, strlen(buffer2), 0);
		
		// receive verification message
		bytes_count = recv(simpleSocket, verification, 256, 0); // get verification for 2nd pair
		if(strcmp(verification, "Verified!") == 0){
			printf("%s\n", verification);
			break;
		}else{
			printf("Invalid receiver name/port number!\n");
		}
	}

	// get connection massage
	bytes_count = recv(simpleSocket, buffer, 256, 0);
	if(bytes_count > 0 && strcmp(buffer, "Receiver Connected!") == 0){
		printf("Relay server connected to receiver!\n");
		while(1){
			char str[25];
			int length = 0;		
			printf("--------------------------------------\n");
			printf("Please enter your messsage: ");
			scanf("%s", buffer);
			if(strcmp(buffer, "x") == 0 ||strcmp(buffer, "q") == 0){
				bytes_count = send(simpleSocket, buffer, strlen(buffer), 0); //send to relay server
				break;
			}else{
				strcat(buffer, "$");
				length = strlen(buffer) + 1; // length of "bla$"
				int extra = 0;
				int len = length;
				while(len/10 != 0){
					len = len/10;
					extra++;
				}
				length = length + extra; // length of "bla$xx"(xx = strlen(str))
				sprintf(str, "%d", length); // integer to string
				strcat(buffer, str);

				// send buffer relay server
				bytes_count = send(simpleSocket, buffer, strlen(buffer), 0);
				bzero(buffer, 256);

				bytes_count = recv(simpleSocket, buffer, 256, 0); // receive from relay server
				printf("%s\n", buffer);
			}
		}
		close(simpleSocket);
	}
 	
    return 0;
}

