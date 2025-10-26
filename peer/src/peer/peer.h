#ifndef PEER_H
#define PEER_H

typedef struct Peer {
  char* name;
  int socketFd;
} Peer;

Peer* createPeer(char*);
int dialPeer(Peer*);

#endif
