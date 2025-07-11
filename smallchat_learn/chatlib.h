#ifndef CHATLIB_H
#define CHATLIB_H

int createTCPServer(int port);
int socketSetNonBlockNoDeploy(int fd);
int acceptClient(int server_socket);
int TCPConnect(char *addr, int port, int nonblockl);

void *chatMalloc(size_t size);
void *chatRealloc(void *ptr , size_t size);

#endif