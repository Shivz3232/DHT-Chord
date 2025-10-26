#include <netdb.h>

#ifndef UTILS_H
#define UTILS_H

// SOCKETS HELPERS
char* getNameInfo(struct sockaddr*, socklen_t*);
int bindToBest(struct addrinfo* addr_info);
void* acceptConnections(void*);

// PACKING & UNPACKING
char* createPacket(char*);
int sendAll(int, char*, int);

char* receivePacket(int);
int receiveAll(int, char*, int);

// CLI ARGS
void* parseArgs(int, char* const*);

#endif
