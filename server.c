#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>

const int NUMBER = 6;
char* username[6];
char* password[6];
char* recvName[6];
char* IPAddress[6];
char* recvPort[6];
char acknowledge[256] = "Acknowledged!";
char verification[256] = "Verified!"; 
char acknowledgeFail[256] = "Acknowledgment Failed!";
char verificationFail[256] = "Verification Failed!"; 
char connect2Recv[256] = "Receiver Connected!";

void readUserList(){
	char buffer[256];
	FILE* fp;
	fp = fopen("userList.txt","r");
	if(fp == NULL){
		fprintf(stderr, "Can't open userList.txt.\n");
	}
	for(int i = 0; i < 6;i++){
		username[i]=(char*)malloc(256*sizeof(char));
		password[i]=(char*)malloc(256*sizeof(char));
	}
	for(int i = 0; i < 6; i++){
		for(int j = 0; j < 2; j++){
			fscanf(fp, "%s", buffer);
			if(j%2 == 0){
				strcpy(username[i],buffer);
			}else{
				strcpy(password[i], buffer);
			}
		}
	}
	fclose(fp);
}

void readReceiverList(){
	char buffer[256];
	FILE* fp;
    	fp = fopen("receiverList.txt", "r");
	if(fp == NULL){
		fprintf(stderr, "Can't open receiverList.txt.\n");
	}
	for(int i = 0; i < 6;i++){
		recvName[i]=(char*)malloc(256*sizeof(char));
		IPAddress[i]=(char*)malloc(256*sizeof(char));
		recvPort[i]=(char*)malloc(256*sizeof(char));
	}
	for(int i = 0; i < 6; i++){
		for(int j = 0; j < 3; j++){
			fscanf(fp, "%s", buffer);
			if(j%3 == 0){
				strcpy(recvName[i],buffer);
			}
			else if(j%3 == 1){
				strcpy(IPAddress[i], buffer);
			}
			else if(j%3 == 2){
				strcpy(recvPort[i], buffer);
			}
		}
	}
	fclose(fp);
}

int login(int simpleChildSocket, char* name, char* passwrd){
	int bytes_count = 0; 
	char buffer[256];
	for(int i = 0; i < 6; i++){
		// if found, send match message
		if(strcmp(username[i],name) == 0 && strcmp(password[i], passwrd) == 0){	
			bytes_count = send(simpleChildSocket, verification, strlen(verification), 0);			
			return 0;				
		}
	}	

	// not found, return failed message
	bytes_count = send(simpleChildSocket, verificationFail, strlen(verificationFail), 0);
	return -1;
}

int verifyRecv(int simpleChildSocket, char* name, char* portNumber){
	int bytes_count = 0; 
	char buffer[256];
	for(int i = 0; i < 6; i++){
		if(strcmp(recvName[i], name) == 0 && strcmp(recvPort[i], portNumber) == 0){
			// send verification
			bytes_count = send(simpleChildSocket, verification, strlen(verification), 0);
			return i; // return the Index of IPAddress				
		}
	}

	// send fail message
	printf("Port number invalid!\n");
	bytes_count = send(simpleChildSocket, verificationFail, strlen(verificationFail), 0);
	return -1;	
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
	if (simpleSocket == -1) {
		fprintf(stderr, "Could not create a socket!\n");
		exit(1);
	}else{
		fprintf(stderr, "Socket created!\n");
   	}
    
	// set up the address structure
	// use INADDR_ANY to bind to all local addresses
	simplePort = atoi(argv[1]);
	bzero(&simpleServer, sizeof(simpleServer));
	simpleServer.sin_family = AF_INET;
	simpleServer.sin_addr.s_addr = htonl(INADDR_ANY);
	simpleServer.sin_port = htons(simplePort);
	
	// bind to the address and port with our socket
	returnStatus = bind(simpleSocket, (struct sockaddr *)&simpleServer, sizeof(simpleServer));
   	if (returnStatus == 0){
    	fprintf(stderr, "Bind completed!\n");
   	}else{
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

	//------------------------------------------------
	readUserList();
	readReceiverList();
   	while(1){
		int index = -1;
		int bytes_count = 0;
   		int flag = -1;
		int idx = -1;
		char buffer[256] = "";
		struct sockaddr_in clientName = { 0 };
		int simpleChildSocket = 0;
		int clientNameLength = sizeof(clientName);

		// wait here
		simpleChildSocket = accept(simpleSocket,(struct sockaddr *)&clientName,&clientNameLength);
		if (simpleChildSocket == -1) {
		    fprintf(stderr, "Cannot accept connections!\n");
		    close(simpleSocket);
		    exit(1);
		}

		//------------------------------------------------------------------------------------		
		// read username, password
		while(1){
			// read username
			bytes_count = recv(simpleChildSocket, buffer, 256, 0);
			if(bytes_count > 0){ 
				bytes_count = send(simpleChildSocket, acknowledge, strlen(acknowledge),0);
			}else{
				printf("Timing out for username!\n");
			}
			
			// read password
			bytes_count = recv(simpleChildSocket, buffer2, 256, 0);		
			flag = login(simpleChildSocket, buffer, buffer2);
			if(flag == 0){
				break;
			}
		}

		// read receiver name, IP address
		while(1){
			// receive receiver name
			bytes_count = recv(simpleChildSocket, buffer, 256, 0);

			// send acknowledge message
			bytes_count = send(simpleChildSocket, acknowledge, strlen(acknowledge), 0);

			// receive port number
			bzero(buffer2, 256);
			bytes_count = recv(simpleChildSocket, buffer2, 256, 0);
			idx = verifyRecv(simpleChildSocket, buffer, buffer2);
			if(idx == -1){
				continue;
			}

			//----------------------------connect to receiver----------------------- 
			int simpleSocket2 = 0;
			int simplePort2 = 0;
			int returnStatus2 = 0; 
			struct sockaddr_in simpleServer2;

			// create a streaming socket
			simpleSocket2 = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (simpleSocket2 == -1){
				fprintf(stderr, "Could not create a socket!\n");
				exit(1);
			}else{
				fprintf(stderr, "Socket created!\n");
			}  

			// set up the address structure
			// use the IP address argument for the server address
			simplePort2 = atoi(recvPort[idx]); // port number
			bzero(&simpleServer2, sizeof(simpleServer2));
			simpleServer2.sin_family = AF_INET;
			inet_addr(argv[1], &simpleServer2.sin_addr.s_addr);
			simpleServer2.sin_port = htons(simplePort2);

			// connect to the address and port with our socket
			returnStatus2 = connect(simpleSocket2, (struct sockaddr *)&simpleServer2, sizeof(simpleServer2));
			if (returnStatus2 == 0) {
				fprintf(stderr, "Connect successful!\n");
				// send "Receiver Connected!" to sender
				bytes_count= send(simpleChildSocket, connect2Recv, strlen(connect2Recv), 0);
			}else{
				fprintf(stderr, "Could not connect to address!\n");
				close(simpleSocket2);
				exit(1);
			}

			// read message from sender
			while(1){
				bzero(buffer, 256); // clear buffer
				bytes_count = recv(simpleChildSocket, buffer, 256, 0); // receive from send
				bytes_count = send(simpleSocket2, buffer, strlen(buffer), 0); // forward to receiver

				bzero(buffer, 256);//clear buffer
				bytes_count = recv(simpleSocket2, buffer, 256, 0); // receive from receiver
				bytes_count = send(simpleChildSocket, buffer, strlen(buffer), 0); // forward to sender
			}
		}
    }

	close(simpleSocket);
	return 0;
}

