#include "testcase.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "../config/config.h"
#include "../logger/logger.h"
#include "../utils/utils.h"

int sendRequest(char*, int, int);

char* executeTestCase(char* operation, int clientId, int objectId) {
  if (sendRequest(operation, clientId, objectId) < 0) {
    info("executeTestCase: Failed to send request\n");
    return NULL;
  }

  char* operationResult = receivePacket(ctx->bootstrap->socketFd);
  if (operationResult == NULL) {
    info("executeTestCase: Failed to receive operation result\n");
    return NULL;
  }

  char* objectIdStr = receivePacket(ctx->bootstrap->socketFd);
  if (objectIdStr == NULL) {
    info("executeTestCase: Failed to receive object id\n");
    return NULL;
  }
  int resultObjectId = atoi(objectIdStr);

  if (resultObjectId != objectId) {
    info("executeTestCase: mismatching object id %d\n", resultObjectId);
    return NULL;
  }

  return operationResult;
}

int sendRequest(char* operation, int clientId, int objectId) {
  char reqIdStr[32];
  snprintf(reqIdStr, 32, "%d", ctx->testCase);

  char clientIdStr[32];
  snprintf(clientIdStr, 32, "%d", clientId);

  char objectIdStr[32];
  snprintf(objectIdStr, 32, "%d", objectId);

  char** data = malloc(sizeof(char*) * 5);
  data[0] = "REQUEST";
  data[1] = operation;
  data[2] = reqIdStr;
  data[3] = objectIdStr;
  data[4] = clientIdStr;

  return createAndSendPackets(ctx->bootstrap->socketFd, data, 5);
}
