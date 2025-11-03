#include <netdb.h>

#include "../peers/peers.h"

#ifndef UTILS_H
#define UTILS_H

// SOCKETS HELPERS
char* getNameInfo(struct sockaddr*, socklen_t*);
int bindToBest(struct addrinfo* addr_info);

// PACKING & UNPACKING
char* createPacket(char*);
int createAndSendPackets(int socketFd, char** data, int size);
int sendPackets(int socketFd, char** packets, int numPackets);
int sendPacket(int, char*);
int sendAll(int, char*, int);

char* receivePacket(int);
int receiveAll(int, char*, int);

// CLI ARGS
void* parseArgs(int, char* const*);

// MISC
char* joinIntegers(int* arr, int count, char sep);

#endif
