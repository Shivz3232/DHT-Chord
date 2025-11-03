#include "../peer/peer.h"

#ifndef CONFIG_H
#define CONFIG_H

typedef struct {
  int socketFd;

  char* name;
  char* network;
  char* inputHostname;
  char* bootstrapHostname;
  int requestDelay;
  int testCase;

  char* cEnv;
  char* hostName;
  char* port;
  int backlog;
  int maxRetries;
  int backoffDuration;

  Peer* bootstrap;

  int maxPeerNameSize;

  int packetHeaderSize;
  int maxMessageSize;
  int maxPacketSize;
} Context;

extern Context* ctx;

void* initializeEnvVariables();

#endif
