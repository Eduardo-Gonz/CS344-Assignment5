#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>  // ssize_t
#include <sys/socket.h> // send(),recv()
#include <netdb.h>      // gethostbyname()

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

int main(int argc, char *argv[]) {
  int socketFD, portNumber, charsWritten, charsRead;
  struct sockaddr_in serverAddress;
  char buffer[256];
  // Check usage & args
  if (argc < 3) { 
    fprintf(stderr,"USAGE: %s hostname port\n", argv[0]); 
    exit(0); 
  } 
  //0 program name
  //1 plaintext
  //2 key
  // port numbr

  // Create a socket
  socketFD = socket(AF_INET, SOCK_STREAM, 0); 
  if (socketFD < 0){
    error("CLIENT: ERROR opening socket");
  }

   // Set up the server address struct
  setupAddressStruct(&serverAddress, atoi(argv[2]), argv[1]);

  //grab contents of plaintext file
    //make sure that the characters in the file are A-Z and space
    //quit program if file isn't correct
  // do the same for the key file

  // Connect to server
  if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
    error("CLIENT: ERROR connecting");
  }

  charsWritten = send(socketFD, "D", 1, 0); 

  if (charsWritten < 0){
    error("CLIENT: ERROR writing to socket");
  }

  charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); 
  printf("%s", buffer);
  if (charsRead < 0){
    error("CLIENT: ERROR reading from socket");
  }
  if (strcmp(buffer, "Denied") == 0) {
    fprintf(stderr, "CLIENT: Connection to server denied\n"); 
    close(socketFD);
    exit(2);
  }



  
  //send the plaintext
    //while loop to send this file
  //send the key
    //while loop to send this file
  //print the encrypted key to stdout
    //while loop to receive what server is sending

  // modify the code underneath
  // Get input message from user
  printf("CLIENT: Enter text to send to the server, and then hit enter: ");
  // Clear out the buffer array
  memset(buffer, '\0', sizeof(buffer));
  // Get input from the user, trunc to buffer - 1 chars, leaving \0
  fgets(buffer, sizeof(buffer) - 1, stdin);
  // Remove the trailing \n that fgets adds
  buffer[strcspn(buffer, "\n")] = '\0'; 

  // Send message to server
  // Write to the server
  charsWritten = send(socketFD, buffer, strlen(buffer), 0); 
  if (charsWritten < 0){
    error("CLIENT: ERROR writing to socket");
  }
  if (charsWritten < strlen(buffer)){
    printf("CLIENT: WARNING: Not all data written to socket!\n");
  }

  // Get return message from server
  // Clear out the buffer again for reuse
  memset(buffer, '\0', sizeof(buffer));
  // Read data from the socket, leaving \0 at end
  charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); 
  if (charsRead < 0){
    error("CLIENT: ERROR reading from socket");
  }
  printf("CLIENT: I received this from the server: \"%s\"\n", buffer);

  // Close the socket
  close(socketFD); 
  return 0;
}

//2 while loops in the server also.