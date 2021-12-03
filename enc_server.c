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
  fflush(stdout);
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


char *encryptTxt(char *plain, char *key) {
  char alphabet[27] = {"ABCDEFGHIJKLMNOPQRSTUVWXYZ "};
  int length = strlen(plain);
  char *encryption = calloc(length + 1, sizeof(char));

  for(int i = 0; i < length; i++) {
    int plainVal = plain[i] - 65;
    if(plain[i] == 32)
      plainVal = 26;
    int keyVal = key[i] - 65;
    if(key[i] == 32)
      keyVal = 26;
    int encryptVal = (plainVal + keyVal) % 27;
    if(encryptVal > 27)
      encryptVal = 0;
    char encrypChar = alphabet[encryptVal];
    strncat(encryption, &encrypChar, 1);
  }
  
  encryption[length] = '+';
  return encryption;
}

int main(int argc, char *argv[]){
  int connectionSocket, charsRead, charsWritten, childStatus;
  char buffer[1000];
  struct sockaddr_in serverAddress, clientAddress;
  socklen_t sizeOfClientInfo = sizeof(clientAddress);
  pid_t spawnPid;

  // Check usage & args
  if (argc < 2) { 
    fprintf(stderr,"USAGE: %s port\n", argv[0]); 
    fflush(stdout);
    exit(1);
  } 
  
  // Create the socket that will listen for connections
  int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (listenSocket < 0) {
    error("ERROR opening socket");
    fflush(stdout);
  }

  // Set up the address struct for the server socket
  setupAddressStruct(&serverAddress, atoi(argv[1]));
  // Associate the socket to the port
  if (bind(listenSocket, 
          (struct sockaddr *)&serverAddress, 
          sizeof(serverAddress)) < 0){
    error("ERROR on binding");
    fflush(stdout);
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
    fflush(stdout);
    spawnPid = fork();
    switch(spawnPid){
          case -1:
              perror("fork()\n");
              fflush(stdout);
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
              fflush(stdout);
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

              int total = 0;
              char allTxt[80000] = {"\0"};
              while(1){
                  memset(buffer, '\0', 1000);
                  if(strchr(allTxt, '}') != NULL)
                    break;
                  charsRead = recv(connectionSocket, buffer, sizeof(buffer) - 1, 0);
                  if (charsRead < 0)
                    error("ERROR reading from socket");
                  if(charsRead == 0){
                    break;
                  }
                  strcat(allTxt, buffer);
                  total += charsRead;
              }

              // Split up the text
              char *saveptr;
              char plain[80000] = {"\0"};
              char keyTxt[80000] = {"\0"};
              // The first token is the plaintext
              char *token = strtok_r(allTxt, "+", &saveptr);
              strcpy(plain, token);

              // The next token is the key
              token = strtok_r(NULL, "}", &saveptr);
              strcpy(keyTxt, token);

              char *encryptedTxt = encryptTxt(plain, keyTxt);
              total = 0; 
              int bytesleft = strlen(encryptedTxt);
              fflush(stdout);

              //Send the encrypted text back to client.
              while(total < strlen(encryptedTxt)) {
                charsWritten = send(connectionSocket, encryptedTxt+total, bytesleft - total, 0);
                if (charsWritten < 0){
                  error("CLIENT: ERROR writing to socket");
                  exit(1);
                }
                if (charsWritten < strlen(buffer)){
                  printf("CLIENT: WARNING: Not all data written to socket!\n");
                }
                total += charsWritten;
                bytesleft -= charsWritten;
              }   

              exit(0);
          default:
              spawnPid = waitpid(spawnPid, &childStatus, WNOHANG);
              break;
    } 
    // Close the connection socket for this client
    close(connectionSocket); 
  }

  // Close the listening socket
  close(listenSocket); 
  return 0;
}