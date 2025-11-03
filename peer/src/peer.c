#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <netdb.h>

#include <pthread.h>

#include "logger/logger.h"
#include "utils/utils.h"
#include "config/config.h"
#include "bootstrap/bootstrap.h"
#include "chord/chord.h"

void* initializeSelfSocket();
int handleNextCommand();
int handlePreviousCommand();

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
  debug("Determining ID.\n");
  extractIdFromObjectStoreFilePath();
  debug("Successfully determined ID to be: %d.\n", ctx->id);
  debug("============================================\n\n\n\n");

  debug("============================================\n");
  debug("Determining ID.\n");
  if (openObjectsFile() < 0) {
    info("Failed to open objects file");
    return 1;
  }
  debug("Successfully determined ID to be: %d.\n", ctx->id);
  debug("============================================\n\n\n\n");

  debug("============================================\n");
  debug("Initializing self socket.\n");
  initializeSelfSocket();
  debug("Successfully initialized self socket.\n");
  debug("============================================\n\n\n\n");

  debug("Sleeping for %d seconds\n", ctx->joinDelay);
  sleep(ctx->joinDelay);
  debug("Woke up\n");

  debug("============================================\n");
  debug("Dialing bootstrap server.\n");
  if (dialBootstrapServer() < 0) {
    debug("Failed to dial bootstrap server\n");
    return 1;
  };
  debug("Successfully conected to bootstrap server.\n");
  debug("============================================\n\n\n\n");

  debug("============================================\n");
  debug("Starting thread to handle requests.\n");
  pthread_t handleRequestsThread;
  pthread_create(&handleRequestsThread, NULL, handleRequests, NULL);
  pthread_detach(handleRequestsThread);
  debug("Successfully started thread to handle requests.\n");
  debug("============================================\n\n\n\n");

  fd_set readFdSet;
  FD_ZERO(&readFdSet);
  FD_SET(ctx->bootstrap->socketFd, &readFdSet);
  while (1) {
    if (select(ctx->bootstrap->socketFd + 1, &readFdSet, NULL, NULL, NULL) < 0) {
      perror("select");
      exit(EXIT_FAILURE);
    }

    if (FD_ISSET(ctx->bootstrap->socketFd, &readFdSet) != 1) {
      info("select returned for an event on unset file descriptor\n");
      continue;
    }

    char buf[1];
    int n = recv(ctx->bootstrap->socketFd, buf, sizeof(buf), MSG_PEEK);
    if (n == 0) {
      debug("Bootstrap closed the connection\n");
      break;
    } else if (n < 0) {
      debug("Failed to MSG_PEEK bootstrap socket\n");
      perror("recv");
      exit(EXIT_FAILURE);
    }

    char* message = receivePacket(ctx->bootstrap->socketFd);
    if (message == NULL) {
      info("Null message from bootstrap server\n");
      continue;
    }

    debug("received %s command from bootstrap\n", message);

    // Handle message;
    if (strcmp(message, "NEXT") == 0) {
      if (handleNextCommand() < 0) {
        info("Failed to handle next command\n");
        break;
      }
    } else if (strcmp(message, "PREVIOUS") == 0) {
      if (handlePreviousCommand() < 0) {
        info("Failed to handle previous command\n");
        break;
      }
    } else if (strcmp(message, "REQUEST") == 0) {
      if (handleRequest(ctx->bootstrap->socketFd) < 0) {
        info("Failed to handle request\n");
        break;
      }
    } else {
      info("Received unknown command from bootstrap server: \"%s\"\n", message);
    }
  }

  debug("============================================\n");
  debug("Wrapping up\n");
  close(ctx->socketFd);
  if (ctx->predecessor != NULL) {
    freePeer(ctx->predecessor);
  }
  if (ctx->successor != NULL) {
    freePeer(ctx->successor);
  }
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

int handleNextCommand() {
  char* idStr = receivePacket(ctx->bootstrap->socketFd);
  if (idStr == NULL) {
    info("handlePreviousCommand: Failed to receive id str\n");
    return -1;
  }

  char* peerName = receivePacket(ctx->bootstrap->socketFd);
  if (peerName == NULL) {
    info("handleNextCommand: Failed to receive next name packet\n");
    return -1;
  }

  if (ctx->successor != NULL) {
    close(ctx->successor->socketFd);
    freePeer(ctx->successor);
  }

  ctx->successor = createPeer(peerName);
  ctx->successor->id = atoi(idStr);

  // if (dialPeer(ctx->successor) < 0) {
  //   info("handleNextCommand: Failed to dial successor %s\n", peerName);
  //   return -1;
  // }

  if (ctx->predecessor != NULL) {
    info("{peer_id:%d, predecesor:%d, succesor:%d}\n", ctx->id, ctx->predecessor->id, ctx->successor->id);
  }

  return 0;
}

int handlePreviousCommand() {
  char* idStr = receivePacket(ctx->bootstrap->socketFd);
  if (idStr == NULL) {
    info("handlePreviousCommand: Failed to receive id str\n");
    return -1;
  }

  char* peerName = receivePacket(ctx->bootstrap->socketFd);
  if (peerName == NULL) {
    info("handlePreviousCommand: Failed to receive next name packet\n");
    return -1;
  }

  if (ctx->predecessor != NULL) {
    close(ctx->predecessor->socketFd);
    freePeer(ctx->predecessor);
  }

  ctx->predecessor = createPeer(peerName);
  ctx->predecessor->id = atoi(idStr);

  // if (dialPeer(ctx->predecessor) < 0) {
  //   info("handlePreviousCommand: Failed to dial predecessor %s\n", peerName);
  //   return -1;
  // }

  if (ctx->successor != NULL) {
    info("{peer_id:%d, predecesor:%d, succesor:%d}\n", ctx->id, ctx->predecessor->id, ctx->successor->id);
  }

  return 0;
}
