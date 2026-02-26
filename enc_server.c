#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>

int MAX_SOCKET_CONNECTIONS = 5;

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
  char textBuffer[256];
  char keyBuffer [256];
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

      memset(textBuffer, '\0', 256);
      textRead = recv(connectionSocket, textBuffer, 255, 0);
      if (textRead < 0){
        error("ERROR reading textfile from socket");
      }
      printf("SERVER: The text file the client is sending is: \"%s\"\n", textBuffer);

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
