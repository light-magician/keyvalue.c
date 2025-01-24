

#include "tcp_server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

TCPServer *create_tcp_server(int port) {
  // How is a client supposed to know what port to try to communicate with?
  // clients typically discover ports throught them being comomon like 80 for
  // HTTP and 443 for HTTPS
}
