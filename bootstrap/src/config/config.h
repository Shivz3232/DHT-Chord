#include "../peers/peers.h"

#ifndef CONFIG_H
#define CONFIG_H

typedef struct {
  int socketFd;

  char* name;
  char* network;
  char* inputHostname;

  char* cEnv;
  int processId;
  char* hostName;
  char* port;
  int backlog;
  int channelSize;
  int maxRetries;
  int backoffDuration;

  Peer* peers;
  int inboundConnections;
  int outboundConnections;

  int maxPeers;
  int maxPeerNameSize;

  int packetHeaderSize;
  int maxMessageSize;
  int maxPacketSize;
} Context;

extern Context* ctx;

void* initializeEnvVariables();

#endif
