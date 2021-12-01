#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>

// Error function used for reporting issues
void error(const char *msg) {
  perror(msg);
  exit(1);
} 

// Set up the address struct for the server socket
void setupAddressStruct(struct sockaddr_in* address, 
                        int portNumber){
 
  // Clear out the address struct
  memset((char*) address, '\0', sizeof(*address)); 

  // The address should be network capable
  address->sin_family = AF_INET;
  // Store the port number
  address->sin_port = htons(portNumber);
  // Allow a client at any address to connect to this server
  address->sin_addr.s_addr = INADDR_ANY;
}

int main(int argc, char *argv[]){
  int connectionSocket, charsRead, childStatus;
  char buffer[1000];
  struct sockaddr_in serverAddress, clientAddress;
  socklen_t sizeOfClientInfo = sizeof(clientAddress);
  pid_t spawnPid;

  // Check usage & args
  if (argc < 2) { 
    fprintf(stderr,"USAGE: %s port\n", argv[0]); 
    exit(1);
  } 
  
  // Create the socket that will listen for connections
  int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (listenSocket < 0) {
    error("ERROR opening socket");
  }

  // Set up the address struct for the server socket
  setupAddressStruct(&serverAddress, atoi(argv[1]));
  // Associate the socket to the port
  if (bind(listenSocket, 
          (struct sockaddr *)&serverAddress, 
          sizeof(serverAddress)) < 0){
    error("ERROR on binding");
  }

  // Start listening for connetions. Allow up to 5 connections to queue up
  listen(listenSocket, 5);

  while(1){
    // Accept the connection request which creates a connection socket
    connectionSocket = accept(listenSocket, 
                (struct sockaddr *)&clientAddress, 
                &sizeOfClientInfo); 
    if (connectionSocket < 0)
      error("ERROR on accept");

    spawnPid = fork();
    switch(spawnPid){
          case -1:
              perror("fork()\n");
              exit(1);
              break;
          case 0:
              //Child Process

              //clear buffer
              memset(buffer, '\0', 1000);
              // Read the client's identifier from the socket
              charsRead = recv(connectionSocket, buffer, 255, 0); 
              if (charsRead < 0)
                error("ERROR reading from socket");

              //Verify identifty of client
              if(strcmp(buffer, "E") != 0) {
                // Send a denied message back to the client
                charsRead = send(connectionSocket, 
                                "Denied", 6, 0); 
                if (charsRead < 0)
                  error("ERROR writing to socket");
                }
              else{
                  // Send a success message back to the client
                charsRead = send(connectionSocket, 
                                "Accepted", 8, 0); 
                if (charsRead < 0)
                  error("ERROR writing to socket");
              }

              //clear buffer
              memset(buffer, '\0', 1000);
              int total = 0;

              while (1){
                  memset(buffer, '\0', 1000);
                  charsRead = recv(connectionSocket, buffer, 1000, 0);
                  if (charsRead < 0)
                    error("ERROR reading from socket");
                  if(charsRead == 0)
                    break;
                  printf("%s\n", buffer);
                  total += charsRead;
              }
                
              //printf("HERE: I received this from the client: \"%s\"\n", buffer);
              break;
          default:
              spawnPid = waitpid(spawnPid, &childStatus, 0);
              break;
    } 
    // Close the connection socket for this client
    close(connectionSocket); 
  }

  // Close the listening socket
  close(listenSocket); 
  return 0;
}