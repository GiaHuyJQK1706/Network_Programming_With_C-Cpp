#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

void handleClient(const string& ip, int port) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        cout << "Socket creation error\n";
        return;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr) <= 0) {
        cout << "Invalid address/ Address not supported\n";
        return;
    }

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        cout << "Connection failed\n";
        return;
    }

    while (true) {
        string message;
        cout << "Enter command (signin/login/logout <UserID> <Password>): ";
        getline(cin, message);

        send(sock, message.c_str(), message.length(), 0);
        int valread = read(sock, buffer, 1024);
        buffer[valread] = '\0';
        cout << "Server: " << buffer << endl;

        if (message == "logout") break;
    }

    close(sock);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: ./client IPAddress PortNumber\n";
        return 1;
    }
    string ip = argv[1];
    int port = stoi(argv[2]);
    handleClient(ip, port);
    return 0;
}
