#include "utils.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "../config/config.h"
#include "../logger/logger.h"

char* getNameInfo(struct sockaddr* peerAddr, socklen_t* peerAddrLen) {
  char* peerName = malloc(NI_MAXHOST);
  if (!peerName) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }

  if (getnameinfo(peerAddr, *peerAddrLen, peerName, ctx->maxPeerNameSize, NULL, 0, NI_NAMEREQD) < 0) {
    perror("getnameinfo");
    exit(EXIT_FAILURE);
  }

  // Reduce FQDN by trimming at the first dot
  char *dot = strchr(peerName, '.');
  if (dot) {
    *dot = '\0';
  }

  return peerName;
}

int bindToBest(struct addrinfo* addrInfo) {
  int yes = 1;

  int socketFd;

  for (struct addrinfo* p = addrInfo; p != NULL; p = p->ai_next) {
    if ((socketFd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) {
      perror("socket");
      continue;
    }

    if (setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) < 0) {
      perror("setsockopt");
      exit(EXIT_FAILURE);
    }

    if (bind(socketFd, p->ai_addr, p->ai_addrlen) < 0) {
      close(socketFd);
      perror("bind");
      continue;
    }

    char host[NI_MAXHOST];
    char service[NI_MAXSERV];
    getnameinfo(addrInfo->ai_addr, addrInfo->ai_addrlen, host, sizeof(host), service, sizeof(service), 0);
    debug("Successfully bound to %s Service: %s\n", host, service);

    return socketFd;
  }

  return -1;
}

char* createPacket(char* content) {
  int contentSize = strlen(content);

  if (contentSize <= 0) {
    debug("createPacket: invalid content, inferred size was zero\n");
    return NULL;
  }

  if (contentSize > ctx->maxPacketSize) {
    debug("Content too large for packet\n");
    return NULL;
  }

  int totalSize = ctx->packetHeaderSize + contentSize;
  char* packet = malloc(totalSize + 1);
  if (packet == NULL) {
    perror("malloc");
    return NULL;
  }

  // Format header as zero-padded ASCII number of fixed width
  // e.g., if ctx->packetHeaderSize = 8, "00000042"
  snprintf(packet, ctx->packetHeaderSize + 1, "%0*d", ctx->packetHeaderSize, contentSize);

  memcpy(packet + ctx->packetHeaderSize, content, contentSize);

  packet[totalSize] = '\0';

  return packet;
}

int sendAll(int socketFd, char* buf, int len) {
  int total = 0;

  int n, bytesleft = len;
  while(total < len) {
    n = send(socketFd, buf + total, bytesleft, 0);
    if (n == -1) {
      perror("send");
      break;
    };

    total += n;
    bytesleft -= n;
  }

  if (n == -1) {
    return -1;
  }

  return total;
}

char* receivePacket(int socketFd) {
  char* header = malloc(ctx->packetHeaderSize + 1);
  if (header == NULL) {
    perror("malloc");
    return NULL;
  }

  if (receiveAll(socketFd, header, ctx->packetHeaderSize) < 0) {
    perror("receiveAll");
    free(header);
    return NULL;
  }

  header[ctx->packetHeaderSize] = '\0';

  char *endptr;
  long inferredContentSize = strtol(header, &endptr, 10);
  if (*endptr != '\0' || inferredContentSize <= 0 || inferredContentSize > ctx->maxPacketSize) {
    info("Invalid packet size in header: %s\n", header);
    return NULL;
  }

  // debug("receivePacket: inferredContentSize: %ld\n", inferredContentSize);

  char* content = malloc(inferredContentSize + 1);
  if (content == NULL) {
    perror("malloc");
    return NULL;
  }

  if (receiveAll(socketFd, content, (int)inferredContentSize) < 0) {
    perror("receiveAll");
    free(content);
    return NULL;
  }

  content[inferredContentSize] = '\0';

  free(header);

  return content;
}

int receiveAll(int socketFd, char* buf, int toBeReceived) {
  for (int received = 0; received != toBeReceived;) {
    int n = recv(socketFd, buf + received, toBeReceived - received, 0);

    if (n < 0) {
      perror("recv");
      return n;
    } else if (n == 0) {
      info("Connection closed while receiving!\n");
      return -1;
    }

    received += n;

    // debug("recv got %d bytes, total %d/%d\n", n, received, toBeReceived);
  }

  return toBeReceived;
}

void* parseArgs(int argc, char* const argv[]) {
  int opt;
  int optionIndex = 0;

  static struct option longOptions[] = {
    {"name",     optional_argument, 0,  0 },
    {"network",  optional_argument, 0,  0 },
    {"hostname", optional_argument, 0,  0 },
    {0,          0,                 0,  0 }
  };

  while (1) {
    debug("here");
    opt = getopt_long(argc, argv, "b:d:o:", longOptions, &optionIndex);

    if (opt == -1) break;

    switch (opt) {
      case 0:
        if (strcmp(longOptions[optionIndex].name, "name") == 0) {
          ctx->name = strdup(optarg);
        } else if (strcmp(longOptions[optionIndex].name, "network") == 0) {
          ctx->network = strdup(optarg);
        } else if (strcmp(longOptions[optionIndex].name, "hostname") == 0) {
          ctx->inputHostname = strdup(optarg);
        }
        break;

      case 'b':
        ctx->bootstrapHostname = strdup(optarg);
        break;

      case 'd':
        ctx->joinDelay = atoi(optarg);
        break;

      case 'o':
        ctx->objectStoreFilePath = strdup(optarg);
        break;

      default:
        break;
    }
  }

  debug("parseArgs: name set to %s", ctx->name);
  debug("parseArgs: network set to %s", ctx->network);
  debug("parseArgs: inputHostname set to %s", ctx->inputHostname);
  debug("parseArgs: bootstrapHostname set to %s", ctx->bootstrapHostname);
  debug("parseArgs: joinDelay set to %d", ctx->joinDelay);
  debug("parseArgs: objectStoreFilePath set to %s", ctx->objectStoreFilePath);

  return NULL;
}
