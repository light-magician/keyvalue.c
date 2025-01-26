

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
#include <pthread.h>

/*
 * address info settings
 * pointer to linked list of server address info
 * socket file descriptor
 * port number
 * the number of connections the server will accept
 * a function pointer for handling generic client reqs
 *  that syntax is a void pointer to a request handler function
 *  that has a void pointer as a return type for flexible
 *  returning behavior
 */
typedef struct {
  struct addrinfo hints;
  struct addrinfo *server_info;
  int socket_fd;
  int port;
  int num_connections;
  void *(*request_handler)(void *client_connection);
} TCPServer;

/*
 * a client file descriptor
 * a way to store socket addresses
 *   of various types IPv4 and IPv6
 *   including the largest socket address types
 * length of a stocket address structure
 * context is an optional parameter that gets passed
 *  to each client handler thread, which allows sharing
 *  data for whatever structures are downstream of the
 *  TCP server (think like whatever facilitates
 *    or whatever else gets done later on)
 */
typedef struct {
  int client_fd;
  struct sockaddr_storage addr;
  socklen_t addr_len;
  void *context;
} ClientConnection;

// core server operations
int tcp_server_init(TCPServer *server, int port, int num_connections,
                    void *(*request_handler)(void *), void *context);
void tcp_server_cleanup(TCPServer *server);
// socker operations
int bind_socket_to_addr(TCPServer *server);
ClientConnection *accept_client_connection(TCPServer *server);
// client handling
int handle_client_request(ClientConnection *client);
void cleanup_client_connection(ClientConnection *client);

#endif
