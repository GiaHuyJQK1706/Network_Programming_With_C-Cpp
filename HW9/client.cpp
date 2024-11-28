#include <iostream>
#include <thread>
#include <string>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <mutex>

using namespace std;

#define BUFFER_SIZE 16384

mutex cout_mutex;

// Nhận tin nhắn từ server
void receiveMessages(int server_socket) {
    char buffer[BUFFER_SIZE];
    while (true) {
        bzero(buffer, BUFFER_SIZE);
        int bytes_received = recv(server_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            lock_guard<mutex> lock(cout_mutex);
            cout << "Server not connect.\n";
            close(server_socket);
            exit(EXIT_FAILURE);
        }
        lock_guard<mutex> lock(cout_mutex);
        cout << buffer << endl;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        cerr << "Using: ./client <IPAddress> <PortNumber>\n";
        return EXIT_FAILURE;
    }

    string server_ip = argv[1];
    int port = stoi(argv[2]);

    // Tạo socket
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        cerr << "Can't create socket\n";
        return EXIT_FAILURE;
    }

    sockaddr_in server_addr = {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr) <= 0) {
        cerr << "IP invalid\n";
        return EXIT_FAILURE;
    }

    // Kết nối tới server
    if (connect(client_socket, (sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        cerr << "Can't connect to server\n";
        return EXIT_FAILURE;
    }
    cout << "Server connect successfully!\n";

    // Đăng nhập hoặc đăng ký
    while (true) {
        string command;
        cout << "Input cmd (login/register username password): ";
        getline(cin, command);
        send(client_socket, command.c_str(), command.length(), 0);

        char response[BUFFER_SIZE];
        bzero(response, BUFFER_SIZE);
        recv(client_socket, response, BUFFER_SIZE, 0);

        if (strcmp(response, "success") == 0) {
            cout << "Login successful!\n";
            break;
        } else if (strcmp(response, "fail") == 0) {
            cout << "Wrong username or password. Try again.\n";
        } else if (strcmp(response, "exist") == 0) {
            cout << "Account already exist! Try again.\n";
        } else {
            cout << "Error! Try again.\n";
        }
    }

    // Bắt đầu nhận tin nhắn
    thread receiver(receiveMessages, client_socket);

    // Gửi tin nhắn
    while (true) {
        string message;
        getline(cin, message);
        send(client_socket, message.c_str(), message.length(), 0);
    }

    receiver.join();
    close(client_socket);
    return 0;
}
