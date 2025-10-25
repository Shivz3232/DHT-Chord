#include "../peers/peers.h"

#ifndef ORCHESTRATE_H
#define ORCHESTRATE_H

typedef struct Node {
  Peer* peer;
  struct Node* next;
  struct Node* previous;
} Node;

void* orchestrate(void*);

#endif
