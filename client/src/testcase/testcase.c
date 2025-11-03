#include "testcase.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "../config/config.h"
#include "../logger/logger.h"
#include "../utils/utils.h"

int sendRequest(char*, int, int);

int testCaseThree() {
  int clientId = 2;
  int objectId = 69;

  int result;
  if ((result = sendRequest("STORE", clientId, objectId)) < 0) {
    info("testCaseThree: Failed to send request\n");
    return result;
  }

  char* operationResult = receivePacket(ctx->bootstrap->socketFd);
  if (operationResult == NULL) {
    info("testCaseThree: Failed to receive operation result\n");
    return -1;
  }

  char* objectIdStr = receivePacket(ctx->bootstrap->socketFd);
  if (objectIdStr == NULL) {
    info("testCaseThree: Failed to receive object id\n");
    return -1;
  }
  int resultObjectId = atoi(objectIdStr);

  if (strcmp(operationResult, "STORED") != 0) {
    info("testCaseThree: unexpected operation result %s\n", operationResult);
    return -1;
  }

  if (resultObjectId != objectId) {
    info("testCaseThree: mismatching object id %d\n", resultObjectId);
    return -1;
  }

  info("%s: %d\n", operationResult, resultObjectId);

  return 0;
}

int testCaseFour() {
  sleep(1000);
  return 0;
}

int testCaseFive() {
  sleep(1000);
  return 0;
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
