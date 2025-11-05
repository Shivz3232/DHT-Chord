#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <netdb.h>

#include <pthread.h>

#include "logger/logger.h"
#include "utils/utils.h"
#include "config/config.h"
#include "orchestrate/orchestrate.h"

void* initializeSelfSocket();
void* initializeAppSocket();
void* app();
int prepareFdSet(fd_set*);
int handleClientRequest(int);

int main(int argc, char const* argv[]) {
  info("Bootstrap process started\n");

  initializeEnvVariables();
  debug("Successfully initialized context.\n");
  debug("============================================\n\n\n\n");

  debug("============================================\n");
  debug("Parsing CLI arguments.\n");
  parseArgs(argc, (char* const*)argv);
  debug("Successfully parsed CLI arguments.\n");
  debug("============================================\n\n\n\n");

  debug("============================================\n");
  debug("Initializing self socket.\n");
  initializeSelfSocket();
  debug("Successfully initialized self socket.\n");
  debug("============================================\n\n\n\n");

  debug("============================================\n");
  debug("Starting orchestration thread.\n");
  pthread_t orchestrationThread;
  pthread_create(&orchestrationThread, NULL, orchestrate, NULL);
  pthread_detach(orchestrationThread);
  debug("Successfully started orchestration thread.\n");
  debug("============================================\n\n\n\n");

  app();

  debug("============================================\n");
  debug("Wrapping up\n");
  close(ctx->socketFd);
  debug("Processs finished\n");
  debug("============================================\n\n\n\n");

  return 0;
}

void* initializeSelfSocket() {
  struct addrinfo hints, *addrInfo;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if (getaddrinfo(NULL, ctx->port, &hints, &addrInfo) < 0) {
    perror("getaddrinfo");
    exit(EXIT_FAILURE);
  }

  debug("============================================\n");
  debug("Creating self socket.\n");
  int socketFd;
  if ((socketFd = bindToBest(addrInfo)) < 0) {
    perror("bindToBest");
    exit(1);
  }
  ctx->socketFd = socketFd;
  debug("Self socket created successfully\n");
  debug("============================================\n\n\n\n");

  debug("============================================\n");
  debug("Trying to listen.\n");
  if (listen(ctx->socketFd, ctx->backlog) < 0) {
    perror("listen");
    exit(EXIT_FAILURE);
  }
  debug("Listening successfully\n");
  debug("============================================\n\n\n\n");

  return NULL;
}

void* initializeAppSocket() {
  struct addrinfo hints, *addrInfo;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if (getaddrinfo(NULL, ctx->appPort, &hints, &addrInfo) < 0) {
    perror("getaddrinfo");
    exit(EXIT_FAILURE);
  }

  debug("============================================\n");
  debug("Creating app socket.\n");
  int appSocketFd;
  if ((appSocketFd = bindToBest(addrInfo)) < 0) {
    perror("bindToBest");
    exit(1);
  }
  ctx->appSocketFd = appSocketFd;
  debug("App socket created successfully\n");
  debug("============================================\n\n\n\n");

  debug("============================================\n");
  debug("Trying to listen.\n");
  if (listen(ctx->appSocketFd, ctx->backlog) < 0) {
    perror("listen");
    exit(EXIT_FAILURE);
  }
  debug("Listening successfully\n");
  debug("============================================\n\n\n\n");

  return NULL;
}

void* app() {
  initializeAppSocket();

  while (1) {
    int clientFd;
    struct sockaddr_storage clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    debug("app: Waiting for connection from client\n");
    if ((clientFd = accept(ctx->appSocketFd, (struct sockaddr*)&clientAddr, &clientAddrLen)) < 0) {
      perror("accept");
      exit(EXIT_FAILURE);
    }

    debug("app: Accepted connection from a client\n");

    char* request = receivePacket(clientFd);
    if (request == NULL) {
      info("app: Failed to receive request from client\n");
      break;
    }

    if (strcmp(request, "REQUEST") != 0) {
      info("app: Unknown request from client: %s\n", request);
      close(clientFd);
      continue;
    }

    if (handleClientRequest(clientFd) < 0) {
      info("app: Failed to handle client request\n");
      close(clientFd);
      break;
    }

    debug("app: Successfully handled client request\n");
  }

  return NULL;
}

int handleClientRequest(int clientFd) {
  char* reqIdStr = receivePacket(clientFd);
  if (reqIdStr == NULL) {
    info("handleClientRequest: Failed to receive request id from client\n");
    return -1;
  }

  char* operationType = receivePacket(clientFd);
  if (operationType == NULL) {
    info("handleClientRequest: Failed to receive operation type from client\n");
    return -1;
  }

  char* objectIdStr = receivePacket(clientFd);
  if (objectIdStr == NULL) {
    info("handleClientRequest: Failed to receive object id from client\n");
    return -1;
  }

  char* clientIdStr = receivePacket(clientFd);
  if (clientIdStr == NULL) {
    info("handleClientRequest: Failed to receive client id from client\n");
    return -1;
  }

  char** data = malloc(sizeof(char*) * 5);
  data[0] = "REQUEST";
  data[1] = operationType;
  data[2] = reqIdStr;
  data[3] = objectIdStr;
  data[4] = clientIdStr;
  if (createAndSendPackets(ctx->ring->peer->socketFd, data, 5) < 0) {
    info("Failed to send request to the ring\n");
    return -1;
  }

  debug("handleClientRequest: Forwarded request to ring\n");

  fd_set fdSet;
  int fdMax = prepareFdSet(&fdSet);

  debug("handleClientRequest: Waiting for a response from the ring\n");
  int result = select(fdMax + 1, &fdSet, NULL, NULL, NULL);
  if (result < 0) {
    perror("select");
    exit(EXIT_FAILURE);
  }

  char* operationResult = "FAILED";
  Node* cur = ctx->ring;
  do {
    if (!FD_ISSET(cur->peer->socketFd, &fdSet)) {
      cur = cur->next;
      continue;
    }

    debug("handleClientRequest: Received response from %s\n", cur->peer->name);

    operationResult = receivePacket(cur->peer->socketFd);
    if (operationResult == NULL) {
      info("handleClientRequest: Failed to receive operation result\n");
      return -1;
    }

    debug("handleClientRequest: Operation result: %s\n", operationResult);

    break;
  } while (cur != ctx->ring);

  char** data2 = malloc(sizeof(char*) * 2);
  data2[0] = operationResult;
  data2[1] = objectIdStr;

  debug("handleClientRequest: Forwarding operation result to client\n");

  return createAndSendPackets(clientFd, data2, 2);
}

int prepareFdSet(fd_set* fdSet) {
  FD_ZERO(fdSet);
  int fdMax = -1;

  Node* cur = ctx->ring;
  do {
    FD_SET(cur->peer->socketFd, fdSet);
    if (cur->peer->socketFd > fdMax) {
      fdMax = cur->peer->socketFd;
    }
    cur = cur->next;
  } while(cur != ctx->ring);

  return fdMax;
}
