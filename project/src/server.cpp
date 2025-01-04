#include "utils/socket_handler.h"
#include "utils/auth_manager.h"
#include "utils/file_manager.h"
#include <iostream>
#include <thread>
#include <vector>
#include <sstream>
#include <cstring>
#include <mutex>
#include <filesystem>
#include <fstream>
#include <cstdlib>
#include <unistd.h>

std::mutex log_mutex;

// Hàm ghi log hoạt động của người dùng
// Tìm file log tương ứng với username, ghi activity
void log_activity(const std::string& username, const std::string& activity) {
    std::lock_guard<std::mutex> guard(log_mutex);
    std::ofstream log_file("db/log/" + username + ".txt", std::ios::app);
    if (log_file.is_open()) {
        log_file << activity << "\n";
        log_file.close();
    } else std::cout << "Something went wrong.\n";
}

// Kiểm tra quyền truy cập của người dùng với file
// Mở file quản lý quyền ứng với người dùng và check
bool has_permission(const std::string& username, const std::string& file_path, const std::string& permission) {
    std::ifstream perm_file("db/perm/" + username + ".txt");
    if (!perm_file.is_open()) return false;

    std::string line, path, perm;
    while (std::getline(perm_file, line)) {
        std::istringstream iss(line);
        iss >> path >> perm;
        if (path == file_path && perm.find(permission) != std::string::npos) return true;
    }
    return false;
}

// Hàm cập nhật quyền của người dùng trên file
void set_permission(const std::string& username, const std::string& file_path, const std::string& new_permission) {
    std::ifstream perm_file_in("db/perm/" + username + ".txt");
    std::ofstream perm_file_out("db/perm/" + username + "_temp.txt");

    bool found = false;
    std::string line, path, perm;
    while (std::getline(perm_file_in, line)) {
        std::istringstream iss(line);
        iss >> path >> perm;
        if (path == file_path) {
            found = true;
            perm_file_out << path << " " << new_permission << "\n";
        } else {
            perm_file_out << line << "\n";
        }
    }
    if (!found) {
        perm_file_out << file_path << " " << new_permission << "\n";
    }
    perm_file_in.close();
    perm_file_out.close();

    std::filesystem::rename("db/perm/" + username + "_temp.txt", "db/perm/" + username + ".txt");
}

// Thêm người dùng mới nếu chưa tồn tại
void register_user(const std::string& username, const std::string& password) {
    std::ofstream user_file("db/users.txt", std::ios::app);
    if (user_file.is_open()) {
        user_file << username << " " << password << "\n";
        user_file.close();
    }
    // Tạo thư mục và các file cấu hình cho người dùng mới
    std::filesystem::create_directory("archive/" + username);
    std::ofstream perm_file("db/perm/" + username + ".txt");
    perm_file << "rw-\n";  // Quyền mặc định là đọc, ghi
    perm_file.close();
    std::ofstream log_file("db/log/" + username + ".txt");
    log_file.close();
}

// Hàm xử lý upload thư mục
void handle_upload_folder(int client_socket, const std::string& username, const std::string& foldername) {
    SocketHandler::send_data(client_socket, "READY"); // ready for folder upload
    try {
        // Create the destination directory
        std::filesystem::path server_folder_path = "archive/" + username + "/" + foldername; 
        std::filesystem::create_directories(server_folder_path); 

        // Receive and save files
        std::string command;
        while (/*(command = SocketHandler::receive_data(client_socket)) != "END_FOLDER"*/1) {
            SocketHandler::send_data(client_socket, "READY"); // ready for folder upload
            command = SocketHandler::receive_data(client_socket);
            if (command == "END_FOLDER") break;
            if (command.substr(0, 4) == "FILE") {
                std::string filename = command.substr(5); // Extract filename from "FILE <filename>"
                std::filesystem::path filepath = server_folder_path / filename; 
                std::ofstream outfile(filepath, std::ios::binary);

                if (!outfile.is_open()) continue;
                SocketHandler::send_data(client_socket, "READY");

                std::string chunk;
                while ((chunk = SocketHandler::receive_data(client_socket)) != "END") {
                    std::cout << chunk << std::endl;
                    outfile.write(chunk.c_str(), chunk.size());
                }
                outfile.close();
                set_permission(username, filepath, "rw-"); // Quyền mặc định là rw-
                log_activity(username, "Uploaded file: " + filename);
            } else {
                std::cerr << "Unexpected command: " << command << std::endl; 
            }
        }

        SocketHandler::send_data(client_socket, "OK"); // Acknowledge successful upload

    } catch (const std::exception& e) {
        std::cerr << "Error handling folder upload: " << e.what() << std::endl;
        SocketHandler::send_data(client_socket, "ERROR"); // Notify client of error
    }
}

// Hàm xử lý download thư mục
void handle_download_folder(int client_socket, const std::string& username) {
    SocketHandler::send_data(client_socket, "Enter folder name to download: ");
    std::string foldername = SocketHandler::receive_data(client_socket);

    std::string folder_path = "archive/" + username + "/" + foldername;
    
    // Kiểm tra thư mục có tồn tại không
    if (!std::filesystem::exists(folder_path)) {
        SocketHandler::send_data(client_socket, "Folder not found.");
        return;
    }

    SocketHandler::send_data(client_socket, "READY");

    // Lặp qua các file trong thư mục và gửi tới client
    for (const auto& entry : std::filesystem::recursive_directory_iterator(folder_path)) {
        if (entry.is_directory()) continue;

        std::string file_path = entry.path().string();
        SocketHandler::send_data(client_socket, "FILE " + file_path);

        std::ifstream file(file_path, std::ios::binary);
        if (!file.is_open()) {
            continue;
        }

        char buffer[1024];
        while (file.read(buffer, sizeof(buffer))) {
            SocketHandler::send_data(client_socket, std::string(buffer, file.gcount()));
        }
        file.close();
        SocketHandler::send_data(client_socket, "END"); // Kết thúc gửi file
    }
    SocketHandler::send_data(client_socket, "END_FOLDER"); // Kết thúc gửi thư mục
}

void handle_client(int client_socket) {
    try {
        std::string command = SocketHandler::receive_data(client_socket);

        if (command == "register") {
            std::string username = SocketHandler::receive_data(client_socket);
            std::string password = SocketHandler::receive_data(client_socket);

            // Kiểm tra nếu người dùng đã tồn tại
            std::ifstream user_file("db/users.txt");
            std::string line;
            bool user_exists = false;
            while (std::getline(user_file, line)) {
                std::istringstream iss(line);
                std::string existing_user;
                iss >> existing_user;
                if (existing_user == username) {
                    user_exists = true;
                    break;
                }
            }

            if (user_exists) {
                SocketHandler::send_data(client_socket, "Username already exists.");
                return;
            } else {
                register_user(username, password);  //
                SocketHandler::send_data(client_socket, "Registration successful.");
                log_activity(username, "Account created.");
                return;
            }
        }

        if (command == "login") {
            std::string username = SocketHandler::receive_data(client_socket);
            std::string password = SocketHandler::receive_data(client_socket);

            if (!AuthManager::authenticate_user(username, password)) {
                SocketHandler::send_data(client_socket, "Authentication failed.");
                log_activity(username, "Authentication failed.");
                close(client_socket);
                return;
            }
            SocketHandler::send_data(client_socket, "Authentication successful.");
            log_activity(username, "Authentication successful.");
            std::string user_dir = "archive/" + username + "/";

            FileManager::create_directory(user_dir);

            while (true) { // after success auth
                std::string command = SocketHandler::receive_data(client_socket);
                if (command.empty()) break;

                std::istringstream iss(command);
                std::string cmd;
                iss >> cmd;

                if (cmd == "upload") {
                    std::string filename;
                    iss >> filename;
                    SocketHandler::send_data(client_socket, "READY");

                    std::ofstream file(user_dir + filename, std::ios::binary);
                    if (!file.is_open()) continue;
                    std::string chunk;
                    while ((chunk = SocketHandler::receive_data(client_socket)) != "END") {
                        file.write(chunk.c_str(), chunk.size());
                    }
                    file.close();
                    set_permission(username, filename, "rw-"); // Quyền mặc định là rw-
                    log_activity(username, "Uploaded file: " + filename);

                } else if (cmd == "download") {
                    std::string filename;
                    iss >> filename;

                    if (!has_permission(username, filename, "r")) {
                        SocketHandler::send_data(client_socket, "Permission denied.");
                        continue;
                    }

                    std::ifstream file(user_dir + filename, std::ios::binary);
                    if (!file.is_open()) {
                        SocketHandler::send_data(client_socket, "File not found.");
                        continue;
                    }
                    SocketHandler::send_data(client_socket, "READY");
                    char buffer[1024];
                    while (file.read(buffer, sizeof(buffer))) {
                        SocketHandler::send_data(client_socket, std::string(buffer, file.gcount()));
                    }
                    file.close();
                    SocketHandler::send_data(client_socket, "END");
                    log_activity(username, "Downloaded file: " + filename);

                } else if (cmd == "upload_folder") {
                    std::string foldername;
                    iss >> foldername;

                    handle_upload_folder(client_socket, username, foldername);
                    log_activity(username, "Uploaded folder " + foldername);

                } else if (cmd == "download_folder") {
                    handle_download_folder(client_socket, username);
                    log_activity(username, "Downloaded folder.");

                } else if (cmd == "set_permission") {
                    std::string filename, new_permission;
                    iss >> filename >> new_permission;

                    if (!has_permission(username, filename, "w")) {
                        SocketHandler::send_data(client_socket, "Permission denied.");
                        continue;
                    }

                    set_permission(username, filename, new_permission);
                    SocketHandler::send_data(client_socket, "Permission updated.");
                    log_activity(username, "Set permission for file: " + filename + " to " + new_permission);

                } else if (cmd == "rename") {
                    std::string old_filename, new_filename;
                    iss >> old_filename >> new_filename;
                    std::filesystem::rename("archive/" + username + "/" + old_filename,
                     "archive/" + username + "/" + new_filename);
                    SocketHandler::send_data(client_socket, "OK");
                } else if (cmd == "delete") {
                    std::string filename;
                    iss >> filename;
                    try {
                        std::filesystem::remove("archive/" + username + "/" + filename); 
                        SocketHandler::send_data(client_socket, "OK"); 
                    } catch (const std::filesystem::filesystem_error& e) { 
                        std::cerr << "Error deleting file: " << e.what() << std::endl; 
                        SocketHandler::send_data(client_socket, "ERROR"); 
                    }
                } else if (cmd == "exit") {
                    SocketHandler::send_data(client_socket, "Goodbye!");
                    break;

                } else {
                    SocketHandler::send_data(client_socket, "Unknown command.");
                }
            }
        }
    } catch (...) {
        std::cerr << "Error handling client.\n";
    }
    close(client_socket);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: ./server <port>\n";
        return -1;
    }

    int port = std::stoi(argv[1]);
    int server_socket = SocketHandler::create_server_socket(port);
    if (server_socket < 0) {
        std::cerr << "Failed to start server.\n";
        return -1;
    }
    std::cout << "Server running on port " << port << "...\n";

    while (true) {
        int client_socket = SocketHandler::accept_connection(server_socket);
        if (client_socket >= 0) {
            std::thread client_thread(handle_client, client_socket);
            client_thread.detach();
        }
    }

    close(server_socket);
    return 0;
}