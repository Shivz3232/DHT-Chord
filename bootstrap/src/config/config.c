#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "../logger/logger.h"

Context* ctx;

void* setDefaults();
void* initializeCEnv();
void* initializePort();
void* initializeHostName();
void* initializeBacklog();
void* initializeChannelSize();
void* initializeMaxRetries();
void* initializeBackoffDuration();
void* initializeMaxPeers();
void* initializeMaxPeerNameSize();
void* initializePacketHeaderSize();
void* initializeMaxMessageSize();
void* initializeMaxPacketSize();

void* initializeEnvVariables() {
  ctx = malloc(sizeof(Context));

  setDefaults();

  initializeCEnv();
  initializePort();
  initializeHostName();
  initializeBacklog();
  initializeChannelSize();
  initializeMaxRetries();
  initializeBackoffDuration();
  initializeMaxPeerNameSize();
  initializeMaxPeers();
  initializePacketHeaderSize();
  initializeMaxMessageSize();
  initializeMaxPacketSize();

  return NULL;
}

void* setDefaults() {
  ctx->cEnv = "development";
  ctx->processId = -1;
  ctx->port = "3000";
  ctx->appPort = "8080";
  ctx->backlog = 10;
  ctx->channelSize = 1000;
  ctx->maxRetries = 5;
  ctx->backoffDuration = 5;

  ctx->inboundConnections = 0;
  ctx->outboundConnections = 0;

  ctx->maxPeers = 7;
  ctx->maxPeerNameSize = 100;

  ctx->packetHeaderSize = 3; // Number of digits in maxMessageSize
  ctx->maxMessageSize = 100;
  ctx->maxPacketSize = ctx->packetHeaderSize + ctx->maxMessageSize;

  return NULL;
}

void* initializeCEnv() {
  char* value = getenv("C_ENV");

  if (!value) {
    debug("C_ENV not found, defaulting to: %s", ctx->cEnv);
  } else {
    ctx->cEnv = strdup(value);
    debug("C_ENV set to %s", ctx->cEnv);
  }

  return NULL;
}

void* initializePort() {
  char* value = getenv("PORT");

  if (!value) {
    debug("PORT not found, defaulting to: %s", ctx->port);
  } else {
    ctx->port = strdup(value);
    debug("PORT set to %s", ctx->port);
  }

  return NULL;
}

void* initializeHostName() {
  char value[ctx->maxPeerNameSize];

  if (gethostname(value, sizeof(value)) != 0) {
    perror("gethostname");
    exit(1);
  }

  ctx->hostName = strdup(value);

  debug("HOSTNAME set to %s", ctx->hostName);

  return NULL;
}

void* initializeBacklog() {
  char* value = getenv("BACKLOG");

  if (!value) {
    debug("BACKLOG not found, defaulting to: %d", ctx->backlog);
  } else {
    ctx->backlog = atoi(value);
    debug("BACKLOG set to %d", ctx->backlog);
  }

  return NULL;
}

void* initializeChannelSize() {
  char* value = getenv("CHANNEL_SIZE");

  if (!value) {
    debug("CHANNEL_SIZE not found, defaulting to: %d", ctx->channelSize);
  } else {
    ctx->channelSize = atoi(value);
    debug("CHANNEL_SIZE set to %d", ctx->channelSize);
  }

  return NULL;
}

void* initializeMaxRetries() {
  char* value = getenv("MAX_RETRIES");

  if (!value) {
    debug("MAX_RETRIES not found, defaulting to: %d", ctx->maxRetries);
  } else {
    ctx->maxRetries = atoi(value);
    debug("MAX_RETRIES set to %d", ctx->maxRetries);
  }

  return NULL;
}

void* initializeBackoffDuration() {
  char* value = getenv("backoffDuration");

  if (!value) {
    debug("backoffDuration not found, defaulting to: %d", ctx->backoffDuration);
  } else {
    ctx->backoffDuration = atoi(value);
    debug("Back-off durationset to %d", ctx->backoffDuration);
  }

  return NULL;
}

void* initializeMaxPeerNameSize() {
  char* value = getenv("MAX_PEER_NAME_SIZE");

  if (!value) {
    debug("MAX_PEER_NAME_SIZE not found, defaulting to: %d", ctx->maxPeerNameSize);
  } else {
    ctx->maxPeerNameSize = atoi(value);
    debug("MAX_PEER_NAME_SIZE set to %d", ctx->maxPeerNameSize);
  }

  return NULL;
}

void* initializeMaxPeers() {
  char* value = getenv("MAX_PEERS");

  if (!value) {
    debug("MAX_PEERS not found, defaulting to: %d", ctx->maxPeers);
  } else {
    ctx->maxPeers = atoi(value);
    debug("MAX_PEERS set to %d", ctx->maxPeers);
  }

  return NULL;
}

void* initializeMaxMessageSize() {
  char* value = getenv("MAX_MESSAGE_SIZE");

  if (!value) {
    debug("MAX_MESSAGE_SIZE not found, defaulting to: %d", ctx->maxMessageSize);
  } else {
    ctx->maxMessageSize = atoi(value);
    debug("MAX_MESSAGE_SIZE set to %d", ctx->maxMessageSize);
  }

  return NULL;
}

void* initializePacketHeaderSize() {
  char* value = getenv("PACKET_HEADER_SIZE");

  if (!value) {
    debug("PACKET_HEADER_SIZE not found, defaulting to: %d", ctx->packetHeaderSize);
  } else {
    ctx->packetHeaderSize = atoi(value);
    debug("PACKET_HEADER_SIZE set to %d", ctx->packetHeaderSize);
  }

  return NULL;
}

void* initializeMaxPacketSize() {
  char* value = getenv("MAX_PACKET_SIZE");

  if (!value) {
    debug("MAX_PACKET_SIZE not found, defaulting to: %d", ctx->maxPacketSize);
  } else {
    ctx->maxPacketSize = atoi(value);
    debug("MAX_PACKET_SIZE set to %d", ctx->maxPacketSize);
  }

  return NULL;
}
