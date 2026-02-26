#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>

int MAX_SOCKET_CONNECTIONS = 5;

char *encryptText(char *plaintextPath, char *keyPath){
  char *plaintextLine = NULL;
  char *keyLine = NULL;
  size_t plaintextLen = 0;
  size_t keyLen = 0;

  FILE *plaintextFile = fopen(plaintextPath, "r");
  FILE *keyFile = fopen(keyPath, "r");

  if (!plaintextFile || !keyFile){
    perror("Error opening files.\n");
    return NULL;
  }

  if (getline(&plaintextLine, &plaintextLen, plaintextFile) == -1){
    perror("Error reading plaintext file.\n");
    return NULL;
  }

  if (getline(&keyLine, &keyLen, keyFile) == -1){
    perror("Error reading plaintext file.\n");
    return NULL;
  }

  plaintextLine[strcspn(plaintextLine, "\n")] = '\0';
  keyLine[strcspn(keyLine, "\n")] = '\0';
  plaintextLen = strlen(plaintextLine);
  keyLen = strlen(keyLine);

  fclose(plaintextFile);
  fclose(keyFile);

  if (keyLen < plaintextLen){
    free(plaintextLine);
    free(keyLine);
    perror("ERROR: Key is shorter than plaintext");
    return NULL;
  }

  char *cipherText = malloc(plaintextLen + 1);
  int cipherLetter;
  int plaintextLetter;
  int keyLetter;

  for (int i = 0; i < plaintextLen; i++){
    if (plaintextLine[i] == ' '){
      plaintextLetter = 26;
    } else {
      plaintextLetter = plaintextLine[i] - 'A';
    }

    if (keyLine[i] == ' '){
      keyLetter = 26;
    } else {
      keyLetter = keyLine[i] - 'A';
    }

    cipherLetter = (keyLetter + plaintextLetter) % 27;
    if (cipherLetter == 26){
      cipherText[i] = ' ';
    } else {
      cipherText[i] = 'A' + cipherLetter;
    }
  }

  cipherText[plaintextLen] = '\0';

  free(plaintextLine);
  free(keyLine);
  return cipherText;

}

void error(const char *msg) {
  perror(msg);
  exit(1);
}

// Set up the address struct for the server socket
void setupAddressStruct(struct sockaddr_in* address, int portNumber){
  memset((char*) address, '\0', sizeof(*address));
  address->sin_family = AF_INET;
  address->sin_port = htons(portNumber);
  address->sin_addr.s_addr = INADDR_ANY;
}

int main(int argc, char *argv[]){
  int connectionSocket, textRead, keyRead;
  char buffer[256];
  char *plaintext;
  char *key;
  struct sockaddr_in serverAddress, clientAddress;
  socklen_t sizeOfClientInfo = sizeof(clientAddress);

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

    pid_t pid = fork();

    if (pid < 0){

      perror("Fork failed");
      close(connectionSocket);
      exit(0);

    } else if (pid == 0){

      printf("SERVER: Connected to client running at host %d port %d\n",
        ntohs(clientAddress.sin_addr.s_addr),
        ntohs(clientAddress.sin_port));

      memset(buffer, '\0', 256);
      textRead = recv(connectionSocket, buffer, 255, 0);
      if (textRead < 0){
        error("ERROR reading textfile from socket");
      }
      printf("SERVER: The text file the client is sending is: \"%s\"\n", buffer);

      plaintext = strtok(buffer, " ");

      key = strtok(NULL, " ");

      printf("SERVER: The plaintext value is:%s: and key value is:%s: \n", plaintext, key);

      char *msg = "I am the server, and I got your message";
      int msg_len = strlen(msg);

      textRead = send(connectionSocket, msg, msg_len, 0);
      if (textRead < 0){
        error("ERROR writing to socket");
      }

      close(connectionSocket);
    } else {
      close(connectionSocket);
      pid_t id = waitpid(-1, NULL, WNOHANG);
    }


  }

  close(listenSocket);
  return 0;
}
