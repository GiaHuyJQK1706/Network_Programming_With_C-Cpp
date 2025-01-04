#include "socket_handler.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

int SocketHandler::create_server_socket(int port) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket creation failed");
        return -1;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        return -1;
    }

    if (listen(server_fd, 5) < 0) {
        perror("Listen failed");
        close(server_fd);
        return -1;
    }

    return server_fd;
}

int SocketHandler::accept_connection(int server_fd) {
    sockaddr_in client_addr{};
    socklen_t addr_len = sizeof(client_addr);
    int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
    if (client_fd < 0) {
        perror("Accept connection failed");
    }
    return client_fd;
}

int SocketHandler::create_client_socket(const std::string& server_ip, int port) {
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0) {
        perror("Socket creation failed");
        return -1;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr);

    if (connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(client_fd);
        return -1;
    }

    return client_fd;
}

bool SocketHandler::send_data(int socket, const std::string& data) {
    ssize_t bytes_sent = send(socket, data.c_str(), data.size(), 0);
    return bytes_sent == (ssize_t)data.size();
}

std::string SocketHandler::receive_data(int socket) {
    char buffer[1024] = {0};
    ssize_t bytes_received = recv(socket, buffer, sizeof(buffer), 0);
    if (bytes_received <= 0) return "";
    return std::string(buffer, bytes_received);
}