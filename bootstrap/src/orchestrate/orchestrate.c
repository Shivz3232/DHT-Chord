#include "orchestrate.h"

#include <stdio.h>
#include <stdlib.h>

#include "../logger/logger.h"
#include "../config/config.h"
#include "../utils/utils.h"
#include "../peers/peers.h"

Node* ring;

Peer* acceptNewConnection();

Node* createNode(Peer*);
void* insertNode(Node*);

int informPeer(Node*);
int informAboutNext(Node*);
int informAboutPrevious(Node*);

void* orchestrate(void* input) {
  while (1) {
    debug("orchestrate: Listening for new connection\n");
    Peer* newPeer = acceptNewConnection();
    if (newPeer == NULL) {
      info("orchestrate: Failed to accept new connection\n");
      continue;
    }

    debug("orchestrate: Accepted connection from peer %s, id: %d\n", newPeer->name, newPeer->id);

    Node* newNode = createNode(newPeer);
    if (newNode == NULL) {
      info("orchestrate: Failed to create a new node for peer %s\n", newPeer->name);
      continue;
    }

    if (insertNode(newNode) < 0) {
      info("orchestrate: Failed to add peer %s to the ring\n", newPeer->name);
      continue;
    }

    debug("orchestrate: Added peer ring, previous: %d, successor: %d\n",
      newNode->previous->peer->id,
      newNode->next->peer->id
    );

    if (informPeer(newNode) < 0) {
      info("orchestrate: Failed to inform new peer %s it's position\n", newNode->peer->name);
      break;
    }

    if (newNode != newNode->previous && informAboutNext(newNode->previous) < 0) {
      info("orchestrate: Failed to inform peer %s's predecessor it's position\n", newNode->peer->name);
      break;
    }

    if (newNode != newNode->next && informAboutPrevious(newNode->next) < 0) {
      info("orchestrate: Failed to inform peer %s's successor it's position\n", newNode->peer->name);
      break;
    }
  }

  info("orchestrate: exiting\n");

  return NULL;
}

Peer* acceptNewConnection() {
  int peerFd;
  struct sockaddr_storage peerAddr;
  socklen_t peerAddrLen = sizeof(peerAddr);

  if ((peerFd = accept(ctx->socketFd, (struct sockaddr*)&peerAddr, &peerAddrLen)) < 0) {
    perror("accept");
    return NULL;
  }

  char* peerName = getNameInfo((struct sockaddr*)&peerAddr, &peerAddrLen);

  Peer* newPeer = createPeer(peerName);
  if (newPeer == NULL) {
    info("acceptNewConnection: Failed to create new peer %s\n", peerName);
    return NULL;
  }

  char* idStr = receivePacket(peerFd);
  if (idStr == NULL) {
    info("acceptNewConnection: Failed to received id str\n");
    freePeer(newPeer);
    return NULL;
  }

  newPeer->id = atoi(idStr);
  newPeer->socketFd = peerFd;

  return newPeer;
}

Node* createNode(Peer* peer) {
  Node* newNode = malloc(sizeof(Node));
  if (newNode == NULL) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }

  newNode->peer = peer;
  newNode->next = NULL;
  newNode->previous = NULL;

  return newNode;
}

void* insertNode(Node* newNode) {
  if (ring == NULL) {
    newNode->next = newNode;
    newNode->previous = newNode;

    ring = newNode;

    return NULL;
  }

  Node* current = ring;
  do {
    if ((current->peer->id < newNode->peer->id && newNode->peer->id < current->next->peer->id) ||
        (current->peer->id > current->next->peer->id && // wrap-around point
         (newNode->peer->id > current->peer->id || newNode->peer->id < current->next->peer->id)
        )
      ) {


      newNode->next = current->next;
      newNode->previous = current;

      current->next->previous = newNode;
      current->next = newNode;

      // If the new ID is smaller than the head’s ID, update head
      if (newNode->peer->id < ring->peer->id) {
        ring = newNode;
      };

      return NULL;
    }

    current = current->next;
  } while (current != ring);

  // If we didn’t find a spot (e.g., all have the same ID order),
  // insert before head (at the end)
  newNode->next = ring;
  newNode->previous = ring->previous;

  ring->previous->next = newNode;
  ring->previous = newNode;

  // If the new peer has a smaller ID than head, it becomes the new head
  if (newNode->peer->id < ring->peer->id) {
    ring = newNode;
  };

  return NULL;
}

int informPeer(Node* node) {
  int result;

  if ((result = informAboutNext(node)) < 0) return result;

  return informAboutPrevious(node);
}

int informAboutNext(Node* node) {
  char* nextCommandPacket = createPacket("NEXT");
  if (nextCommandPacket == NULL) {
    info("informAboutNext: Failed to create next command packet\n");
    return -1;
  }

  char* nextNamePacket = createPacket(node->next->peer->name);
  if (nextNamePacket == NULL) {
    info("informAboutNext: Failed to create next name packet\n");
    return -1;
  }

  int numBytesSent;
  if ((numBytesSent = sendPacket(node->peer->socketFd, nextCommandPacket)) < 0) {
    info("informAboutNext: sendPacket: Failed to send next command packet. numBytesSent: %d\n", numBytesSent);
    return -1;
  }

  if ((numBytesSent = sendPacket(node->peer->socketFd, nextNamePacket)) < 0) {
    info("informAboutNext: sendPacket: Failed to send next name packet. numBytesSent: %d\n", numBytesSent);
    return -1;
  }

  return 0;
}

int informAboutPrevious(Node* node) {
  char* previousCommandPacket = createPacket("PREVIOUS");
  if (previousCommandPacket == NULL) {
    info("informAboutPrevious: Failed to create previous command packet\n");
    return -1;
  }

  char* previousNamePacket = createPacket(node->previous->peer->name);
  if (previousNamePacket == NULL) {
    info("informAboutPrevious: Failed to create previous name packet\n");
    return -1;
  }

  int numBytesSent;
  if ((numBytesSent = sendPacket(node->peer->socketFd, previousCommandPacket)) < 0) {
    info("informAboutPrevious: sendPacket: Failed to send previous command packet. numBytesSent: %d\n", numBytesSent);
    return -1;
  }

  if ((numBytesSent = sendPacket(node->peer->socketFd, previousNamePacket)) < 0) {
    info("informAboutPrevious: sendPacket: Failed to send previous name packet. numBytesSent: %d\n", numBytesSent);
    return -1;
  }

  return 0;
}
