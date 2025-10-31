#ifndef PEER_H
#define PEER_H

typedef struct Peer {
  int id;
  char* name;
  int socketFd;
} Peer;

Peer* createPeer(char*);
int dialPeer(Peer*);
void* freePeer(Peer*);

#endif
