#include "chord.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "../config/config.h"
#include "../logger/logger.h"
#include "../utils/utils.h"

int handleStore(char* reqIdStr, int predecessorFd);
int handleRetreive(char* reqIdStr, int predecessorFd);

int forwardRequest(char*, char*, char*, char*);

void* handleRequests(void* input) {
  int peerFd = -1;
  struct sockaddr_storage peerAddr;
  socklen_t peerAddrLen = sizeof(peerAddr);

  while (1) {
    if ((peerFd = accept(ctx->socketFd, (struct sockaddr*)&peerAddr, &peerAddrLen)) < 0) {
      perror("accept");
      exit(EXIT_FAILURE);
    }

    char* peerName = getNameInfo((struct sockaddr*)&peerAddr, &peerAddrLen);
    if ((ctx->predecessor != NULL && strcmp(peerName, ctx->predecessor->name) != 0)) {
      info("handleRequests: Recieved connection from a non predecessor %s, current predecessor: %s\n", peerName, ctx->predecessor->name);
      close(peerFd);
      continue;
    }

    debug("handleRequests: received connection from peer %s\n", peerName);

    if (handleRequest(peerFd) < 0) {
      info("handleRequests: Failed to handle request from predecessor\n");
      break;
    }

    debug("handleRequests: Successfully handled request\n");
  }

  if (peerFd != -1) {
    close(peerFd);
    freeaddrinfo((struct addrinfo*) &peerAddr);
  }

  return NULL;
}

int handleRequest(int predecessorFd) {
  char* reqIdStr = receivePacket(predecessorFd);
  if (reqIdStr == NULL) {
    info("handleRequest: Failed to receive request id from predecessor\n");
    return -1;
  }

  char* operation = receivePacket(predecessorFd);
  if (operation == NULL) {
    info("handleRequest: Failed to receive operation from predecessor\n");
    return -1;
  }

  debug("handleRequest: Handling %s request with id %s\n", operation, reqIdStr);

  if (strcmp(operation, "STORE") == 0) {
    return handleStore(reqIdStr, predecessorFd);
  } else if (strcmp(operation, "RETRIEVE") == 0) {
    return handleRetreive(reqIdStr, predecessorFd);
  }

  info("handleRequest: received unknown operation from predecessor: %s\n", operation);

  return -1;
}

int handleStore(char* reqIdStr, int predecessorFd) {
  char* objectIdStr = receivePacket(predecessorFd);
  if (objectIdStr == NULL) {
    info("handleStore: Failed to receive object id string\n");
    return -1;
  }

  char* clientIdStr = receivePacket(predecessorFd);
  if (clientIdStr == NULL) {
    info("handleStore: Failed to receive client id string\n");
    return -1;
  }

  debug("handleStore: Handling store request with id %s for object %s from client %s\n", reqIdStr, objectIdStr, clientIdStr);

  if (atoi(objectIdStr) > ctx->id) {
    if (ctx->successor->id > ctx->id) {
      debug("handleStore: Forwarding request\n");
      return forwardRequest(reqIdStr, "STORE", objectIdStr, clientIdStr);
    } else {
      debug("handleStore: Storing with self\n");
    }
  }

  if (fprintf(ctx->objectsFile, "%s::%s\n", clientIdStr, objectIdStr) >= 0) {
    debug("handleStore: Successfully stored object\n");
  } else {
    info("handleStore: Failed to store object\n");
    return -1;
  }

  char* commandPacket = createPacket("STORED");
  if (commandPacket == NULL) {
    info("handleStore: Failed to create command packet\n");
    return -1;
  }

  int numBytesSent;
  if ((numBytesSent = sendPacket(ctx->bootstrap->socketFd, commandPacket)) < 0) {
    debug("handleStore :sendPacket: Failed to send command packet. numBytesSent: %d\n", numBytesSent);
    return -1;
  }

  return 0;
}

int handleRetreive(char* reqIdStr, int predecessorFd) {
  char* objectIdStr = receivePacket(predecessorFd);
  if (objectIdStr == NULL) {
    info("handleStore: Failed to receive object id string\n");
    return -1;
  }

  char* clientIdStr = receivePacket(predecessorFd);
  if (clientIdStr == NULL) {
    info("handleStore: Failed to receive client id string\n");
    return -1;
  }

  if (atoi(objectIdStr) > ctx->id) {
    return forwardRequest(reqIdStr, "RETRIEVE", objectIdStr, clientIdStr);
  }

  int requestedClientId = atoi(clientIdStr);
  int requestedObjectId = atoi(objectIdStr);

  int found = 0;
  int clientId = -1;
  int objectId = -1;
  while (fscanf(ctx->objectsFile, "%d::%d\n", &clientId, &objectId) == 2) {
    if (clientId == requestedClientId && objectId == requestedObjectId) {
      found = 1;
      break;
    }
  }
  rewind(ctx->objectsFile);

  char** data = malloc(sizeof(char*) * 1);
  if (found == 1) {
    data[0] = "RETRIEVED";
  } else {
    data[0] = "NOTFOUND";
  }

  return createAndSendPackets(ctx->socketFd, data, 1);
}

int forwardRequest(char* reqId, char* operation, char* objectIdStr, char* clientIdStr) {
  char** data = malloc(sizeof(char*) * 4);
  data[0] = reqId;
  data[1] = operation;
  data[2] = objectIdStr;
  data[3] = clientIdStr;

  int socketFd = dialPeer(ctx->successor);
  if (socketFd < 0) {
    info("forwardRequest: Failed to dial successor\n");
    return socketFd;
  }

  int r = createAndSendPackets(socketFd, data, 4);
  if (r < 0) {
    info("forwardRequest: Failed to forward request\n");
  }

  close(socketFd);

  return r;
}
