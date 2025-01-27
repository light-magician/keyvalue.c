

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

static int init_epoll(TCPServer *server) {
  server->epoll_fd = epoll_create1(0);
  if (server->epoll_fd < 0)
    return -1;

  struct epoll_event ev = {.events = EPOLLIN | EPOLLET,
                           .data.fd = server->socket_fd};

  return epoll_ctl(server->epoll_fd, EPOLL_CTL_ADD, server->socket_fd, &ev);
}

static int handle_new_connection(TCPServer *server) {
  ClientConnection *client = malloc(sizeof(ClientConnection));
  client->addr_len = sizeof(client->addr);

  client->client_fd = accept(
      server->socket_fd, (struct sockaddr *)&client->addr, &client->addr_len);

  if (client->client_fd < 0) {
    if (errno != EAGAIN && errno != EWOULDBLOCK) {
      fprintf(stderr, "accept error\n");
    }
    free(client);
    return -1;
  }

  make_socket_non_blocking(client->client_fd);

  struct epoll_event ev = {.events = EPOLLIN | EPOLLET, .data.ptr = client};

  if (epoll_ctl(server->epoll_fd, EPOLL_CTL_ADD, client->client_fd, &ev) < 0) {
    close(client->client_fd);
    free(client);
    return -1;
  }

  return 0;
}

static int event_loop(TCPServer *server) {
  struct epoll_event events[MAX_EVENTS];

  while (1) {
    int nfds = epoll_wait(server->epoll_fd, events, MAX_EVENTS, -1);
    if (nfds < 0) {
      if (errno == EINTR)
        continue;
      return -1;
    }

    for (int i = 0; i < nfds; i++) {
      if (events[i].data.fd == server->socket_fd) {
        while (handle_new_connection(server) == 0)
          ;
      } else {
        server->handle_event((ClientConnection *)events[i].data.ptr,
                             events[i].events);
      }
    }
  }
}

int tcp_server_start(TCPServer *server) {
  if (!server->handle_event)
    return -1;
  if (server->port <= 0)
    server->port = 8080;
  if (server->backlog <= 0)
    server->backlog = 10;

  if (bind_socket(server) < 0)
    return -1;
  if (init_epoll(server) < 0)
    return -1;
  if (listen(server->socket_fd, server->backlog) < 0)
    return -1;

  return event_loop(server);
}

void tcp_server_stop(TCPServer *server) {
  if (server->epoll_fd >= 0) {
    close(server->epoll_fd);
    server->epoll_fd = -1;
  }

  if (server->socket_fd >= 0) {
    close(server->socket_fd);
    server->socket_fd = -1;
  }
}
