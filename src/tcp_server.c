

#include "tcp_server.h"
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

// client connection being handled
// date shared accross client connections
typedef struct {
  ClientConnection *client;
  void *shared_data;
} ThreadContext;
// configuration to make socket non blocking

int bind_socket(TCPServer *server) {
  // search through address structures for a match to bind socket to
  struct addrinfo hints = {0}, *server_info, *addr;
  char port_str[6];
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  snprintf(port_str, sizeof(port_str), "%d", server->port);

  if (getaddrinfo(NULL, port_str, &hints, &server_info) != 0) {
    return -1;
  }

  int set_reuse = 1;
  // linked list iteration, set new addr pointer to head of list
  //  server->info and progress to ai_next until that is null
  for (addr = server_info; addr != NULL; addr = addr->ai_next) {
    server->socket_fd =
        socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
    if (server->socket_fd < 0) {
      continue;
    }
    if (setsockopt(server->socket_fd, SOL_SOCKET, SO_REUSEADDR, &set_reuse,
                   sizeof(set_reuse)) < 0) {
      close(server->socket_fd);
      continue;
    }
    if (bind(server->socket_fd, addr->ai_addr, addr->ai_addrlen) == 0) {
      make_socket_non_blocking(server->socket_fd);
      break; // Successfully bound
    }
    close(server->socket_fd);
  }
  // can actually free server_info here as the socket is bound
  freeaddrinfo(server_info);
  // pass or fail depends on if addr has a value
  return (addr != NULL) ? 0 : -1;
}
