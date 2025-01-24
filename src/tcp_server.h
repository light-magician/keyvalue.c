

/*
 * Making the worlds smallest key value store
 * that runs on any computer with a network interface card.
 */

#ifndef TCP_SERVER_H
#define TCP_SERVER_H

// higher level network database operations
//  like getaddrinfo(), gethostbyname(), and
//  protocol independent name resolution
#include <netdb.h>

typedef struct {
  struct addrinfo hints;
  struct addrinfo *server_info;
  int socket_fd;
  int port;
  int num_connections;
} TCPServer;

typedef void (*ClientHandler)(int client_fd, void *context);
int bind_socket_to_addr(TCPServer *server);
int tcp_server_init(TCPServer *server, int port, int num_connections);
int tcp_server_start(TCPServer *server, ClientHandler handler, void *context);
void tcp_server_cleanup(TCPServer *server);

#endif
