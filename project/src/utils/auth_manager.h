#ifndef AUTH_MANAGER_H
#define AUTH_MANAGER_H

#include <string>
#include <map>

class AuthManager {
public:
    static bool register_user(const std::string& username, const std::string& password);
    static bool authenticate_user(const std::string& username, const std::string& password);
};

#endif