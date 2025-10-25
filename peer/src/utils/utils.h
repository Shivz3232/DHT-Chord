#include <netdb.h>

#include "../peers/peers.h"

#ifndef UTILS_H
#define UTILS_H

void* dialBootstrapServer();

// SOCKETS HELPERS
char* getNameInfo(struct sockaddr*, socklen_t*);
int bindToBest(struct addrinfo* addr_info);

// PACKING & UNPACKING
char* createPacket(char*);
int sendAll(int, char*, int);

char* receivePacket(int);
int receiveAll(int, char*, int);

// CLI ARGS
void* parseArgs(int, char* const*);

#endif
