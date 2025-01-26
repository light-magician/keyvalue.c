

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
// event driven polling
#include <sys/epoll.h>

#define MAX_EVENTS 64
#define BUFFER_SIZE 4096

/* A Client connectoin only has client socket details
 *
 * a client file descriptor
 * a way to store socket addresses
 *   of various types IPv4 and IPv6
 *   including the largest socket address types
 * length of a stocket address structure
 */
typedef struct {
  int client_fd;
  struct sockaddr_storage addr;
  socklen_t addr_len;
  char buffer[BUFFER_SIZE];
  size_t bytes_processed;
} ClientConnection;
/* Server handles only networking and threading
 *
 * socket file descriptor
 * epoll instance for event driven model
 * port number
 * backlog is max number of pending connections
 * a function pointer for handling generic client reqs
 *  that syntax is a void pointer to a request handler function
 *  that has a void pointer as a return type for flexible
 *  returning behavior
 */
typedef struct {
  int socket_fd;
  int epoll_fd;
  int port;
  int backlog;
  void (*handle_request)(ClientConnection *client);
} TCPServer;
// core functions
int tcp_server_start(TCPServer *server);
void tcp_server_stop(TCPServer *server);

#endif
