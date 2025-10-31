#include "peer.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

Peer* createPeer(char* name) {
  Peer* newPeer = malloc(sizeof(Peer));
  if (newPeer == NULL) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }

  newPeer->name = name;

  return newPeer;
}

int dialPeer(Peer* peer) {
  return 0;
}

void* freePeer(Peer* peer) {
  free(peer->name);
  free(peer);
  return NULL;
}
