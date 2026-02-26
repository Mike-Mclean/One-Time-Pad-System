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
  char *cipherText = NULL;
  char *keyFileString = NULL;
  size_t len_ciphertext = 0;
  size_t len_key = 0;

  if (argc < 4) {
    fprintf(stderr,"USAGE: %s cyphertext key port\n", argv[0]);
    exit(0);
  }

  FILE *cipherTextFile = fopen(argv[1], "r");
  FILE *keyFile = fopen(argv[2], "r");

  if (!cipherTextFile || !keyFile){
    error("Error opening files.\n");
  }

  if (getline(&cipherText, &len_ciphertext, cipherTextFile) == -1){
    error("Error reading cipher text");
  }

  if (getline(&keyFileString, &len_key, keyFile) == -1){
    error("Error reading key text");
  }

  len_ciphertext = strlen(cipherText);
  len_key = strlen(keyFileString);

  fclose(cipherTextFile);
  fclose(keyFile);

  if (len_key < len_ciphertext){
    free(cipherText);
    free(keyFileString);
    error("Error: Key is too short");
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
      error("Error: Bad character in plaintext received");
    }
  }

  printf("%s\n", buffer);

  close(socketFD);
  return 0;
}