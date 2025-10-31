#include <netdb.h>

#include "../peer/peer.h"

#ifndef UTILS_H
#define UTILS_H

// SOCKETS HELPERS
char* getNameInfo(struct sockaddr*, socklen_t*);
int bindToBest(struct addrinfo* addr_info);

// PACKING & UNPACKING
char* createPacket(char*);
int sendAll(int, char*, int);
int sendPacket(int, char*);

char* receivePacket(int);
int receiveAll(int, char*, int);

// CLI ARGS
void* parseArgs(int, char* const*);

#endif
