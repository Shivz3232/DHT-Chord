#include "../peer/peer.h"

#ifndef CONFIG_H
#define CONFIG_H

typedef struct {
  int id;
  FILE* objectsFile;

  int socketFd;

  char* name;
  char* network;
  char* inputHostname;
  char* bootstrapHostname;
  int joinDelay;
  char* objectStoreFilePath;

  char* cEnv;
  int processId;
  char* hostName;
  char* port;
  int backlog;
  int channelSize;
  int maxRetries;
  int backoffDuration;
  int maxObjectsFilePathSize;
  int maxObjectsFileLineSize;

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
