#ifndef SOCKET_HANDLER_H
#define SOCKET_HANDLER_H

#include <string>
#include <netinet/in.h>

class SocketHandler {
public:
    static int create_server_socket(int port);
    static int accept_connection(int server_fd);
    static int create_client_socket(const std::string& server_ip, int port);
    static bool send_data(int socket, const std::string& data);
    static std::string receive_data(int socket);
};

#endif