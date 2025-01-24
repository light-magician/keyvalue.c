

#include "tcp_server.h"
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int bind_socket_to_addr(TCPServer *server) {
  // search through address structures for a match to bind socket to
  struct addrinfo *addr;
  int set_reuse = 1;
  // linked list iteration, set new addr pointer to head of list
  //  server->info and progress to ai_next until that is null
  for (addr = server->server_info; addr != NULL; addr = addr->ai_next) {
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
      break; // Successfully bound
    }
    close(server->socket_fd);
  }
  // verify address
  if (addr == NULL) {
    fprintf(stderr, "failed to bind to any address\n");
    freeaddrinfo(server->server_info);
    return -1;
  }
  // listen for connections
  if (listen(server->socket_fd, server->num_connections) < 0) {
    close(server->socket_fd);
    freeaddrinfo(server->server_info);
    return -1;
  }
  return 0;
}

int tcp_server_init(TCPServer *server, int port, int num_connections) {
  memset(server, 0, sizeof(TCPServer));
  memset(&server->hints, 0, sizeof(server->hints));
  // set address information
  server->hints.ai_family = AF_UNSPEC;     // IPv4 and IPv6 support
  server->hints.ai_socktype = SOCK_STREAM; // set TCP
  server->hints.ai_flags = AI_PASSIVE;     // wildcard IP addresses
  server->port = port;
  server->num_connections = num_connections;
  // convert port int to string for getaddrinfo for any int size wo overflow
  char port_str[6];
  snprintf(port_str, sizeof(port_str), "%d", port);
  int status =
      getaddrinfo(NULL, port_str, &server->hints, &server->server_info);
  if (status != 0) {
    fprintf(stderr, "get address info error: %s\n", gai_strerror(status));
    return -1;
  }
  if (bind_socket_to_addr(server) < 0) {
    fprintf(stderr, "server initialization failed");
    return -1;
  }
  printf("server initialized");
  return 0;
}
