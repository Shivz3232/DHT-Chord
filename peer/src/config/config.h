#include "../peer/peer.h"

#ifndef CONFIG_H
#define CONFIG_H

typedef struct {
  int socketFd;

  char* name;
  char* network;
  char* inputHostname;
  char* bootstrapHostname;

  char* cEnv;
  int processId;
  char* hostName;
  char* port;
  int backlog;
  int channelSize;
  int maxRetries;
  int backoffDuration;

  Peer* bootstrap;
  Peer* predecessor;
  Peer* successor;

  int maxPeerNameSize;

  int packetHeaderSize;
  int maxMessageSize;
  int maxPacketSize;
} Context;

extern Context* ctx;

void* initializeEnvVariables();

#endif
