#include <iostream>
#include <fstream>
#include <list>
#include <string>
#include <cstring>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

using namespace std;

struct Account {
    string userID;
    string password;
    int status;
    int failedAttempts;
};

list<Account> accounts;

void loadAccounts(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Unable to open account file.\n";
        exit(1);
    }
    string userID, password;
    int status;
    while (file >> userID >> password >> status) {
        accounts.push_back({userID, password, status, 0});
    }
    file.close();
}

void saveAccounts(const string& filename) {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Unable to save account file.\n";
        exit(1);
    }
    for (const auto& account : accounts) {
        file << account.userID << " " << account.password << " " << account.status << endl;
    }
    file.close();
}

list<Account>::iterator findAccount(const string& userID) {
    for (auto it = accounts.begin(); it != accounts.end(); ++it) {
        if (it->userID == userID) return it;
    }
    return accounts.end();
}

string handleRequest(const string& request) {
    string command, userID, password;
    size_t pos = request.find(' ');
    command = request.substr(0, pos);

    if (command == "signin") {
        size_t pos2 = request.find(' ', pos + 1);
        userID = request.substr(pos + 1, pos2 - pos - 1);
        password = request.substr(pos2 + 1);
        if (findAccount(userID) == accounts.end()) {
            accounts.push_back({userID, password, 1, 0});
            saveAccounts("account.txt");
            return "Account created successfully.";
        }
        return "Account already exists.";
    } else if (command == "login") {
        size_t pos2 = request.find(' ', pos + 1);
        userID = request.substr(pos + 1, pos2 - pos - 1);
        password = request.substr(pos2 + 1);

        auto it = findAccount(userID);
        if (it == accounts.end()) return "User not found.";
        if (it->status == 0) return "Account is locked.";
        if (it->password == password) {
            it->failedAttempts = 0;
            return "Login successful.";
        }
        it->failedAttempts++;
        if (it->failedAttempts >= 3) {
            it->status = 0;
            saveAccounts("account.txt");
            return "Account is locked.";
        }
        return "Wrong password.";
    } else if (command == "logout") {
        return "Logout successful.";
    }
    return "Invalid command.";
}

void handleServer(int port) {
    int server_fd, new_socket, activity, valread, sd, max_sd;
    struct sockaddr_in address;
    int opt = 1, addrlen = sizeof(address), client_socket[30], max_clients = 30;
    char buffer[1025];
    fd_set readfds;

    for (int i = 0; i < max_clients; i++) client_socket[i] = 0;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) < 0) {
        perror("Setsockopt failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    cout << "Server is running on port " << port << endl;

    while (true) {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        max_sd = server_fd;

        for (int i = 0; i < max_clients; i++) {
            sd = client_socket[i];
            if (sd > 0) FD_SET(sd, &readfds);
            if (sd > max_sd) max_sd = sd;
        }

        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR)) {
            cout << "Select error.\n";
        }

        if (FD_ISSET(server_fd, &readfds)) {
            if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
                perror("Accept");
                exit(EXIT_FAILURE);
            }

            cout << "New connection: socket fd " << new_socket << endl;

            for (int i = 0; i < max_clients; i++) {
                if (client_socket[i] == 0) {
                    client_socket[i] = new_socket;
                    break;
                }
            }
        }

        for (int i = 0; i < max_clients; i++) {
            sd = client_socket[i];
            if (FD_ISSET(sd, &readfds)) {
                valread = read(sd, buffer, 1024);
                if (valread == 0) {
                    close(sd);
                    client_socket[i] = 0;
                } else {
                    buffer[valread] = '\0';
                    string response = handleRequest(string(buffer));
                    send(sd, response.c_str(), response.size(), 0);
                }
            }
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: ./server PortNumber\n";
        return 1;
    }
    int port = stoi(argv[1]);
    loadAccounts("account.txt");
    handleServer(port);
    saveAccounts("account.txt");
    return 0;
}
