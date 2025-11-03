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
  initializeMaxPeerNameSize();
  initializePacketHeaderSize();
  initializeMaxMessageSize();
  initializeMaxPacketSize();

  return NULL;
}

void* setDefaults() {
  ctx->cEnv = "production";
  ctx->port = "8080";
  ctx->backlog = 1;
  ctx->maxRetries = 5;
  ctx->backoffDuration = 5;

  ctx->maxPeerNameSize = 100;

  ctx->packetHeaderSize = 3;
  ctx->maxMessageSize = 100;

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
  ctx->maxPacketSize = ctx->packetHeaderSize + ctx->maxMessageSize;
  return NULL;
}
