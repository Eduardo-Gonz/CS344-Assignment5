#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>  // ssize_t
#include <sys/socket.h> // send(),recv()
#include <netdb.h>      // gethostbyname()
#include <fcntl.h>
#include <stdint.h>
#include <dirent.h>

/**
* Client code
* 1. Create a socket and connect to the server specified in the command arugments.
* 2. Prompt the user for input and send that input as a message to the server.
* 3. Print the message received from the server and exit the program.
*/

// Error function used for reporting issues
void error(const char *msg) { 
  perror(msg); 
  exit(0); 
} 

// Set up the address struct
void setupAddressStruct(struct sockaddr_in* address, 
                        int portNumber, 
                        char* hostname){
 
  // Clear out the address struct
  memset((char*) address, '\0', sizeof(*address)); 

  // The address should be network capable
  address->sin_family = AF_INET;
  // Store the port number
  address->sin_port = htons(portNumber);

  // Get the DNS entry for this host name
  struct hostent* hostInfo = gethostbyname(hostname); 
  if (hostInfo == NULL) { 
    fprintf(stderr, "CLIENT: ERROR, no such host\n"); 
    exit(0); 
  }
  // Copy the first IP address from the DNS entry to sin_addr.s_addr
  memcpy((char*) &address->sin_addr.s_addr, 
        hostInfo->h_addr_list[0],
        hostInfo->h_length);
}

char *grabFile(char *fileName) {
  char * line = NULL;
	size_t len = 0;
	ssize_t read;

  FILE *fp = fopen(fileName, "r");
	if (fp == NULL)
		exit(1);

	read = getline(&line, &len, fp);
	fclose(fp);

  return line;
}

int validateFile(char *fileTxt) {
  //make sure text is made up of alphabet characters or spaces.
  int i = 0;
  while(fileTxt[i] != '\0') {

    if( (fileTxt[i] >= 'A' && fileTxt[i] <= 'Z') || fileTxt[i] == ' '){
      i++;
      continue;
    }
    fflush(stdout);
    
    return -1;
 }

  return 1;
}

void removeNewLine(char *txt) {
  int length = strlen(txt);
  txt[length - 1] = '\0';
}

void addSeperator(char *txt) {
  int length = strlen(txt);
  txt[length] = '+';
}

int main(int argc, char *argv[]) {
  int socketFD, portNumber, charsWritten, charsRead;
  struct sockaddr_in serverAddress;
  char buffer[256];

  // Check usage & args
  if (argc < 4) { 
    fprintf(stderr,"USAGE: %s plaintext key port\n", argv[0]);
    fflush(stdout);
    exit(0); 
  } 
  
  //grab text needed for encryption and remove trailing \n
  char *plainTxt = grabFile(argv[1]);
  char *keyTxt = grabFile(argv[2]);
  removeNewLine(plainTxt);
  removeNewLine(keyTxt);

  //validate plainTxt and keyTxt
  if(validateFile(plainTxt) == -1 || validateFile(keyTxt) == -1) {
    fprintf(stderr, "Invalid characters in one or both files\n");
    free(plainTxt);
    free(keyTxt);
    fflush(stdout);
    exit(1);
  }
  if(strlen(plainTxt) > strlen(keyTxt)){
    fprintf(stderr, "Key must be as long or longer than plain text");
    free(plainTxt);
    free(keyTxt);
    fflush(stdout);
    exit(1);
  }

  addSeperator(plainTxt);

  // Create a socket
  socketFD = socket(AF_INET, SOCK_STREAM, 0); 
  if (socketFD < 0){
    error("CLIENT: ERROR opening socket");
  }

   // Set up the server address struct
  setupAddressStruct(&serverAddress, atoi(argv[3]), "localhost");

  //Send an identifer to ensure client connects to correct server
  // Connect to server
  if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
    error("CLIENT: ERROR connecting");
  }

  charsWritten = send(socketFD, "E", 1, 0); 

  if (charsWritten < 0){
    error("CLIENT: ERROR writing to socket");
  }

  charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); 

  if (charsRead < 0){
    error("CLIENT: ERROR reading from socket");
  }
  if (strcmp(buffer, "Denied") == 0) {
    fprintf(stderr, "CLIENT: Connection to server denied\n");
    fflush(stdout);
    free(plainTxt);
    free(keyTxt);
    close(socketFD);
    exit(2);
  }

  size_t total = 0;        // how many bytes we've sent
  //size_t bytesleft = strlen(allTxt); // how many we have left to send
  char *allTxt = calloc(strlen(plainTxt) + strlen(keyTxt) + 1, sizeof(char));
  strcat(allTxt, plainTxt);
  strcat(allTxt, keyTxt);
  strcat(allTxt, "}");
  size_t bytesleft = strlen(allTxt);

  while(total < strlen(allTxt)) {
    charsWritten = send(socketFD, allTxt+total, bytesleft-total, 0);
    if (charsWritten < 0){
      error("CLIENT: ERROR writing to socket");
    }
    if (charsWritten < strlen(buffer)){
      printf("CLIENT: WARNING: Not all data written to socket!\n");
    }
    total += charsWritten;
    bytesleft -= charsWritten;
  }


  char encryptedTxt[80000] = {"\0"};
  char encryptBuffer[1000];
  charsRead = 0;

  fflush(stdout);
  while(1){
    memset(encryptBuffer, '\0', 1000);
    if(strchr(encryptedTxt, '+') != NULL)
      break;
    charsRead = recv(socketFD, encryptBuffer, sizeof(encryptBuffer) - 1, 0);
    if (charsRead < 0)
      error("ERROR reading from socket");
    if(charsRead == 0){
      break;
    }
    strcat(encryptedTxt, encryptBuffer);
    total += charsRead;
  }
  encryptedTxt[strlen(encryptedTxt) - 1] = '\n';

  printf("%s", encryptedTxt);

  // Close the socket and free allocated memory
  close(socketFD); 
  free(plainTxt);
  free(keyTxt);
  free(allTxt);
  return 0;
}