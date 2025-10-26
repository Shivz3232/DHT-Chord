#include "bootstrap.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "../logger/logger.h"
#include "../config/config.h"

void* dialBootstrapServer() {
  ctx->bootstrap = malloc(sizeof(Peer));
  if (!ctx->bootstrap) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }

  ctx->bootstrap->name = strdup(ctx->bootstrapHostname);

  int retries = 0;
  int connected = 0;
  while (connected == 0 && retries < ctx->maxRetries) {
    struct addrinfo hints, *addrInfo, *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(ctx->bootstrap->name, ctx->port, &hints, &addrInfo) < 0) {
      perror("getaddrinfo");
      retries += 1;
      sleep(ctx->backoffDuration);
      debug("Retrying to connect to %s\n", ctx->bootstrap->name);
      continue;
    }

    int yes = 1;
    int socketFd;
    for (p = addrInfo; p != NULL; p = p->ai_next) {
      if ((socketFd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) {
        perror("socket");
        continue;
      }

      debug("Attempting connection to %s\n", ctx->bootstrap->name);

      if (connect(socketFd, p->ai_addr, p->ai_addrlen) < 0) {
        perror("connect");
        close(socketFd);
        continue;
      }

      debug("Successful outbound connection to %s\n", ctx->bootstrap->name);

      break;
    }

    if (!p) {
      info("Failed to connect to %s. Tried %d times. Sleeping.\n", ctx->bootstrap->name, retries + 1);
      retries += 1;

      if (retries == ctx->maxRetries) {
        info("dialBootstrapServer: Failed to dial bootstrap\n");
        exit(1);
      } else {
        sleep(ctx->backoffDuration);
        debug("Retrying to connect to %s\n", ctx->bootstrap->name);
        continue;
      }
    }

    ctx->bootstrap->socketFd  = socketFd;

    connected = 1;
  }

  return NULL;
}
