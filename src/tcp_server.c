

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
static int make_socket_non_blocking(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  if (flags == -1)
    return -1;
  return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

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

int *handle_client_thread(void *arg) {
  // argo should end up a client
  ClientConnection *client = (ClientConnection *)arg;
  // handle if server has a defined handle function
  if (server->handle_request) {
    server->handle_request(client);
  }
  // cleanup client once handled
  close(client->client_fd);
  free(client);
  return NULL;
}

ClientConnection *accept_client_connection(TCPServer *server) {
  // heap allocate a client and return a pointer to it
  ClientConnection *client = malloc(sizeof(ClientConnection));
  if (!client) {
    return NULL;
  }
  // zero out memory to prevent undefined behavior
  memset(client, 0, sizeof(ClientConnection));
  // not a pointer to client so no need for ->
  client->addr_len = sizeof(client->addr);
  client->client_fd = accept(
      server->socket_fd, (struct sockaddr *)&client->addr, &client->addr_len);
  if (client->client_fd < 0) {
    free(client);
    return NULL;
  }
  printf("server accepted new client connection\n");
  return client;
}

void *handle_client_thread(void *arg) {
  // create a client connection out of the arg
  ClientConnection *client = (ClientConnection *)arg;
  // server handles handle req
  server->handle_request(client);
  close(client->client_fd);
  free(client);
  return NUll; // TODO: maybe we return success here
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
  // listen for connections
  if (listen(server->socket_fd, server->num_connections) < 0) {
    close(server->socket_fd);
    freeaddrinfo(server->server_info);
    return -1;
  }
  printf("server initialized");

  while (1) {
    ClientConnection *client = accept_client_connection(server);
    if (client->client_fd < 0) {
      continue;
    }
    handle_client_request(client);
    close(client->client_fd);
    free(client);
  }
  printf("server terminated");
  return 0;
}
