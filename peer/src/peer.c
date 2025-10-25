#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <netdb.h>

#include <pthread.h>

#include "logger/logger.h"
#include "utils/utils.h"
#include "config/config.h"

void* initializeSelfSocket();

int main(int argc, char const* argv[]) {
  info("Peer process started\n");

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
  parseArgs(argc, (char* const*)argv);
  debug("Successfully initialized self socket.\n");
  debug("============================================\n\n\n\n");

  initializeSelfSocket();

  debug("============================================\n");
  debug("Dialing bootstrap server.\n");
  dialBootstrapServer();
  debug("Successfully conected to bootstrap server.\n");
  debug("============================================\n\n\n\n");

  fd_set readFdSet;
  FD_ZERO(&readFdSet);
  FD_SET(ctx->bootstrap->socketFd, &readFdSet);
  while (1) {
    if (select(ctx->bootstrap->socketFd + 1, readFdSet, NULL, NULL, NULL) < 0) {
      perror("select");
      exit(EXIT_FAILURE);
    }

    if (FD_ISSET(ctx->bootstrap->socketFd, &readFdSet) != 1) {
      info("select returned for an event on unset file descriptor\n");
      continue;
    }

    char* message = receivePacket(ctx->bootstrap->socketFd);
    if (message == NULL) {
      info("Null message from bootstrap server\n");
      continue;
    }

    // Handle message;
  }

  debug("============================================\n");
  debug("Wrapping up\n");
  close(ctx->socketFd);
  freeaddrinfo(addrInfo);
  freePeers(ctx->peers);
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
