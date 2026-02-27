#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <netdb.h>
#include <ctype.h>

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
  char ack[10];

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

  memset(buffer, '\0', sizeof(buffer));
  strncpy(buffer, "ENC", sizeof(buffer) - 1);

  charsWritten = send(socketFD, buffer, strlen(buffer), 0);
  if (charsWritten < 0){
    error("CLIENT: ERROR writing to socket");
  }
  if (charsWritten < strlen(buffer)){
    error("CLIENT: WARNING: Not all data written to socket!\n");
  }

  charsRead = recv(socketFD, ack, sizeof(ack) - 1, 0);
  if (charsRead < 0 || charsRead == 0){
    error("CLIENT: Error verifying server/client connection");
  }

  memset(buffer, '\0', sizeof(buffer));

  snprintf(buffer, sizeof(buffer), "%s %s", argv[1], argv[2]);

  charsWritten = send(socketFD, buffer, strlen(buffer), 0);
  if (charsWritten < 0){
    error("CLIENT: ERROR writing to socket");
  }
  if (charsWritten < strlen(buffer)){
    printf("CLIENT: WARNING: Not all data written to socket!\n");
  }

  charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0);
  if (charsRead < 0){
    error("CLIENT: ERROR reading from socket");
  }

  for (int i; buffer[i] != '\0'; i++){
    if (isalpha(buffer[i]) == 0 && buffer[i] != ' '){
      error("Error: Bad character in cipher received");
    }
  }

  printf("%s\n", buffer);

  close(socketFD);
  return 0;
}