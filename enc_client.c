#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <netdb.h>

int MAX_CONNECTIONS = 5;

void error(const char *msg){
    perror(msg);
    exit(1);
}

void setupAddressStruct(struct sockaddr_in* address, int portNumber){
    memset((char*) address, '\0', sizeof(*address));
    address->sin_family = AF_INET;
    address->sin_port = htons(portNumber);

    address->sin_addr.s_addr = htonl(INADDR_LOOPBACK);
}

int main(int argc, char *argv[]) {
  int socketFD, charsWritten, charsRead;
  struct sockaddr_in serverAddress;
  char buffer[256];

  if (argc < 4) {
    fprintf(stderr,"USAGE: %s plaintext key port\n", argv[0]);
    exit(0);
  }

  socketFD = socket(AF_INET, SOCK_STREAM, 0);
  if (socketFD < 0){
    error("CLIENT: ERROR opening socket");
  }

  setupAddressStruct(&serverAddress, atoi(argv[3]));

  if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
    error("CLIENT: ERROR connecting");
  }

  printf("CLIENT: Enter text to send to the server, and then hit enter: ");

  memset(buffer, '\0', sizeof(buffer));

  fgets(buffer, sizeof(buffer) - 1, stdin);
  buffer[strcspn(buffer, "\n")] = '\0';

  charsWritten = send(socketFD, buffer, strlen(buffer), 0);
  if (charsWritten < 0){
    error("CLIENT: ERROR writing to socket");
  }
  if (charsWritten < strlen(buffer)){
    printf("CLIENT: WARNING: Not all data written to socket!\n");
  }

  memset(buffer, '\0', sizeof(buffer));

  charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0);
  if (charsRead < 0){
    error("CLIENT: ERROR reading from socket");
  }
  printf("CLIENT: I received this from the server: \"%s\"\n", buffer);

  close(socketFD);
  return 0;
}