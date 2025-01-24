

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

/*
 * address info settings
 * pointer to linked list of server address info
 * socket file descriptor
 * port number
 * the number of connections the server will accept
 */
typedef struct {
  struct addrinfo hints;
  struct addrinfo *server_info;
  int socket_fd;
  int port;
  int num_connections;
} TCPServer;

/*
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
} ClientConnection;

ClientConnection *accept_client_connection(TCPServer *server);
int handle_client_request(ClientConnection *client);
int bind_socket_to_addr(TCPServer *server);
int tcp_server_init(TCPServer *server, int port, int num_connections);
void tcp_server_cleanup(TCPServer *server);

#endif
