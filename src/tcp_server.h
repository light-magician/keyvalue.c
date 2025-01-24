

/*
 * Making the worlds smallest key value store
 * that runs on any computer with a network interface card.
 */

#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <netinet/in.h>

typedef struct {
  int socket_fd;
  struct sockaddr_in address;
  int port;
} TCPServer;

typedef void (*ClientHandler)(int client_fd, void *context);
TCPServer *init_tcp_server(TCPServer *server, int port);
int tcp_server_start(TCPServer *server, ClientHandler handler, void *context);
void tcp_server_destroy(TCPServer *server);

#endif
