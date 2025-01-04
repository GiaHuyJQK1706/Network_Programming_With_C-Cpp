#include "auth_manager.h"
#include <fstream>
#include <sstream>

bool AuthManager::register_user(const std::string& username, const std::string& password) {
    std::ofstream user_file("db/users.txt", std::ios::app);
    if (!user_file.is_open()) return false;

    user_file << username << " " << password << "\n";
    user_file.close();
    return true;
}

bool AuthManager::authenticate_user(const std::string& username, const std::string& password) {
    std::ifstream user_file("db/users.txt");
    if (!user_file.is_open()) return false;

    std::string line, u, p;
    while (std::getline(user_file, line)) {
        std::istringstream iss(line);
        iss >> u >> p;
        if (u == username && p == password) return true;
    }
    return false;
}