#include <iostream>
#include <thread>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <string>
#include <mutex>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>

using namespace std;

#define BUFFER_SIZE 16384

mutex client_mutex;
map<int, string> clients; // Map lưu socket và username
map<string, string> accounts; // Lưu thông tin tài khoản từ file

// Tải thông tin tài khoản từ file account.txt
void loadAccounts(const string &filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Not open file account.txt\n";
        exit(EXIT_FAILURE);
    }
    string line, username, password;
    while (getline(file, line)) {
        stringstream ss(line);
        ss >> username >> password;
        accounts[username] = password;
    }
    file.close();
}

// Gửi tin nhắn tới tất cả các client (broadcast)
void broadcastMessage(const string &message, int sender) {
    lock_guard<mutex> lock(client_mutex);
    for (const auto &[sock, username] : clients) {
        if (sock != sender) {
            send(sock, message.c_str(), message.length(), 0);
        }
    }
}

// Xử lý client
void handleClient(int client_socket) {
    char buffer[BUFFER_SIZE];
    string username;

    // Đăng nhập hoặc đăng ký
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        recv(client_socket, buffer, BUFFER_SIZE, 0);
        string command(buffer);
        stringstream ss(command);
        string action, user, pass;
        ss >> action >> user >> pass;

        if (action == "login") {
            if (accounts.count(user) && accounts[user] == pass) {
                send(client_socket, "success", 7, 0);
                username = user;
                break;
            } else {
                send(client_socket, "fail", 4, 0);
            }
        } else if (action == "register") {
            if (!accounts.count(user)) {
                accounts[user] = pass;
                ofstream file("account.txt", ios::app);
                file << user << " " << pass << endl;
                file.close();
                send(client_socket, "success", 7, 0);
                username = user;
                break;
            } else {
                send(client_socket, "exist", 5, 0);
            }
        }
    }

    // Thêm client vào danh sách
    {
        lock_guard<mutex> lock(client_mutex); 
        clients[client_socket] = username;
    }

    // Lắng nghe tin nhắn từ client
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            lock_guard<mutex> lock(client_mutex);
            clients.erase(client_socket);
            close(client_socket);
            break;
        }

        string message(buffer);
        if (message.find('>') != string::npos) {
            size_t pos = message.find('>');
            string recipient = message.substr(0, pos);
            string content = message.substr(pos + 1);

            lock_guard<mutex> lock(client_mutex);
            bool found = false;
            for (const auto &[sock, uname] : clients) {
                if (uname == recipient) {
                    string formatted_message = "(" + username + ": " + content + ")";
                    send(sock, formatted_message.c_str(), formatted_message.length(), 0);
                    found = true;
                    break;
                }
            }
            if (!found) {
                string error_message = "User " + recipient + " not exist!";
                send(client_socket, error_message.c_str(), error_message.length(), 0);
            }
        } else {
            string error_message = "Syntax error! Using: <username>> <messenge>. Ex: user_abc> mess_send";
            send(client_socket, error_message.c_str(), error_message.length(), 0);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        cerr << "Using: ./server <PortNumber>\n";
        return EXIT_FAILURE;
    }

    int port = stoi(argv[1]);
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        cerr << "Can't create socket\n";
        return EXIT_FAILURE;
    }

    sockaddr_in server_addr = {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        cerr << "Bind error\n";
        return EXIT_FAILURE;
    }

    if (listen(server_socket, 10) < 0) {
        cerr << "listen error\n";
        return EXIT_FAILURE;
    }

    cout << "Server running on port: " << port << "\n";
    loadAccounts("account.txt");

    vector<thread> threads;
    while (true) {
        sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_socket = accept(server_socket, (sockaddr *)&client_addr, &client_len);
        if (client_socket < 0) {
            cerr << "Accept error\n";
            continue;
        }
        threads.emplace_back(handleClient, client_socket);
    }

    for (auto &t : threads) {
        t.join();
    }

    close(server_socket);
    return 0;
}