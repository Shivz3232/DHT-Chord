#include <netdb.h>

#ifndef PEERS_H
#define PEERS_H

typedef struct Peer {
  int id;
  char* name;
  int socketFd;
} Peer;

Peer* createPeer(char*);
void* freePeer(Peer*);

#endif
