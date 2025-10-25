#include "peers.h"

#include <stdio.h>
#include <stdlib.h>

Peer* createPeer(char* name) {
  Peer* newPeer = malloc(sizeof(Peer));
  if (!newPeer) {
    perror("malloc");
    return NULL;
  }

  newPeer->id = -1;
  newPeer->name = name;
  newPeer->socketFd = -1;

  return newPeer;
}
