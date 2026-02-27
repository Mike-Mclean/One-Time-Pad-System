#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>

int MAX_SOCKET_CONNECTIONS = 5;

char *decryptText(char *ciphertextPath, char *keyPath){
  char *ciphertextLine = NULL;
  char *keyLine = NULL;
  size_t ciphertextLen = 0;
  size_t keyLen = 0;

  FILE *ciphertextFile = fopen(ciphertextPath, "r");
  FILE *keyFile = fopen(keyPath, "r");

  if (!ciphertextFile || !keyFile){
    perror("Error opening files.\n");
    return NULL;
  }

  if (getline(&ciphertextLine, &ciphertextLen, ciphertextFile) == -1){
    perror("Error reading plaintext file.\n");
    return NULL;
  }

  if (getline(&keyLine, &keyLen, keyFile) == -1){
    perror("Error reading plaintext file.\n");
    return NULL;
  }

  ciphertextLine[strcspn(ciphertextLine, "\n")] = '\0';
  keyLine[strcspn(keyLine, "\n")] = '\0';
  ciphertextLen = strlen(ciphertextLine);
  keyLen = strlen(keyLine);

  fclose(ciphertextFile);
  fclose(keyFile);

  if (keyLen < ciphertextLen){
    free(ciphertextLine);
    free(keyLine);
    perror("ERROR: Key is shorter than plaintext");
    return NULL;
  }

  char *plainletterText = malloc(ciphertextLen + 1);
  int cipherLetter;
  int plaintextLetter;
  int keyLetter;

  for (int i = 0; i < ciphertextLen; i++){
    if (ciphertextLine[i] == ' '){
      cipherLetter = 26;
    } else {
      cipherLetter = ciphertextLine[i] - 'A';
    }

    if (keyLine[i] == ' '){
      keyLetter = 26;
    } else {
      keyLetter = keyLine[i] - 'A';
    }

    plaintextLetter = (cipherLetter - keyLetter + 27) % 27;

    if (plaintextLetter == 26){
      plainletterText[i] = ' ';
    } else {
      plainletterText[i] = 'A' + plaintextLetter;
    }
  }

  plainletterText[ciphertextLen] = '\0';

  free(ciphertextLine);
  free(keyLine);
  return plainletterText;

}

void error(const char *msg) {
  perror(msg);
  exit(1);
}

void setupAddressStruct(struct sockaddr_in* address, int portNumber){
  memset((char*) address, '\0', sizeof(*address));
  address->sin_family = AF_INET;
  address->sin_port = htons(portNumber);
  address->sin_addr.s_addr = INADDR_ANY;
}

int main(int argc, char *argv[]){
  int connectionSocket, textRead, keyRead;
  char buffer[256];
  char ack[10];
  char *ciphertext;
  char *key;
  struct sockaddr_in serverAddress, clientAddress;
  socklen_t sizeOfClientInfo = sizeof(clientAddress);
  int num_connections = 0;

  if (argc < 2) {
    fprintf(stderr,"USAGE: %s port\n", argv[0]);
    exit(1);
  }

  int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (listenSocket < 0) {
    error("ERROR opening socket");
  }

  setupAddressStruct(&serverAddress, atoi(argv[1]));

  if (bind(listenSocket,
          (struct sockaddr *)&serverAddress,
          sizeof(serverAddress)) < 0){
    error("ERROR on binding");
  }

  listen(listenSocket, MAX_SOCKET_CONNECTIONS);

  while(1){
    connectionSocket = accept(listenSocket,
                (struct sockaddr *)&clientAddress,
                &sizeOfClientInfo);
    if (connectionSocket < 0){
      error("ERROR on accept");
    }

    num_connections++;
    pid_t pid = fork();

    if (pid < 0){

      perror("Fork failed");
      close(connectionSocket);
      exit(0);

    } else if (pid == 0){

      memset(buffer, '\0', 256);
      textRead = recv(connectionSocket, buffer, 255, 0);
      if (textRead < 0){
        error("ERROR reading textfile from socket");
      }

      ciphertext = strtok(buffer, "\0");

      if (strcmp(ciphertext, "DEC") != 0){
        error("SERVER: Verification key mismatch! Closing connection.\n");
        close(connectionSocket);
        continue;
      }

      send(connectionSocket, "ACK", sizeof(ack) - 1, 0);

      memset(buffer, '\0', 256);
      textRead = recv(connectionSocket, buffer, 255, 0);
      if (textRead < 0){
        error("ERROR reading textfile from socket");
      }

      ciphertext = strtok(buffer, " ");

      key = strtok(NULL, " ");

      char *plaintext = decryptText(ciphertext, key);
      if (plaintext == NULL){
        perror("ERROR: text could not be encrypted.\n");
        close(connectionSocket);
        continue;
      }

      textRead = send(connectionSocket, plaintext, strlen(plaintext), 0);
      if (textRead < 0){
        error("ERROR writing to socket");
      }
      close(connectionSocket);
      exit(0);
    } else {
      close(connectionSocket);
      pid_t id = waitpid(-1, NULL, WNOHANG);
      while (id > 0){
        num_connections--;
        id = waitpid(-1, NULL, WNOHANG);
      }
    }

  }

  close(listenSocket);
  return 0;
}
