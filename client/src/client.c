#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <netdb.h>

#include <pthread.h>

#include "config/config.h"
#include "logger/logger.h"
#include "utils/utils.h"
#include "bootstrap/bootstrap.h"
#include "testcase/testcase.h"

void* initializeSelfSocket();

int main(int argc, char const* argv[]) {
  info("Process started\n");

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

  debug("Going to sleep for %d seconds\n", ctx->requestDelay);
  sleep(ctx->requestDelay);
  debug("Woke up\n");

  debug("============================================\n");
  debug("Connect to bootstrap.\n");
  if (dialBootstrapServer() < 0) {
    info("Failed to dial bootstrap\n");
    return 1;
  }
  debug("Successfully connected to bootstrap.\n");
  debug("============================================\n\n\n\n");

  debug("============================================\n");
  debug("Executing testcase %d.\n", ctx->testCase);
  char *operation, *operationResult, *expectedOperationResult;
  int clientId, objectId;
  switch (ctx->testCase) {
    case 3:
      operationResult = executeTestCase(
        (operation = "STORE"),
        (clientId = 2),
        (objectId = 69)
      );
      expectedOperationResult = "STORED";
      break;

    case 4:
      operationResult = executeTestCase(
        (operation = "RETRIEVE"),
        (clientId = 3),
        (objectId = 126)
      );
      expectedOperationResult = "RETRIEVED";
      break;

    case 5:
      operationResult = executeTestCase(
        (operation = "RETRIEVE"),
        (clientId = 2),
        (objectId = 69)
      );
      expectedOperationResult = "NOT FOUND";
      break;

    case 6:
      operationResult = executeTestCase(
        (operation = "RETRIEVE"),
        (clientId = 2),
        (objectId = 69)
      );
      expectedOperationResult = "RETRIEVED";
      break;

    default:
      info("Unknown testcase\n");
      break;
  }

  if (operationResult == NULL) {
    info("Failed to execute testcase\n");
    debug("============================================\n\n\n\n");
  }

  if (strcmp(operationResult, expectedOperationResult) == 0) {
    info("%s: %d\n", operationResult, objectId);
    info("Successfully executed testcase\n");
    debug("============================================\n\n\n\n");
  } else {
    info("Failed to execute testcase\n");
    info("Unexpected operation result %s, expected %s\n", operationResult, expectedOperationResult);
    debug("============================================\n\n\n\n");
  }

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
