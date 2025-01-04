#include "utils/socket_handler.h"
#include "utils/auth_manager.h"
#include "utils/file_manager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <vector>
#include <cstring>
#include <filesystem>
#include <unistd.h>

namespace fs = std::filesystem;

// Hàm gửi thư mục lên server
void handle_upload_folder(int server_socket) {
    std::string foldername;
    std::cout << "Enter folder name to upload: ";
    std::cin >> foldername;

    // Kiểm tra thư mục có tồn tại không
    if (!fs::exists(foldername) || !fs::is_directory(foldername)) {
        std::cout << "Folder not found.\n";
        return;
    }

    // Gửi lệnh upload_folder tới server
    SocketHandler::send_data(server_socket, "upload_folder " + foldername);
    
    // Duyệt qua tất cả các file trong thư mục và gửi
    for (const auto& entry : fs::recursive_directory_iterator(foldername)) {
        std::string response = SocketHandler::receive_data(server_socket);
        if (response != "READY") {
            std::cout << "Server not ready for folder upload.\n";
            return;
        }
        if (entry.is_directory()) continue; // Bỏ qua thư mục con

        std::string relative_path = fs::relative(entry.path(), foldername).string();
        SocketHandler::send_data(server_socket, "FILE " + relative_path);
        std::ifstream file(entry.path(), std::ios::binary);
        if (!file.is_open()) {
            std::cout << "Failed to open file: " << entry.path().string() << "\n";
            continue;
        }

        response = SocketHandler::receive_data(server_socket);
        if (response != "READY") {
            std::cout << "Server not ready for file upload.\n";
            return;
        }

        char buffer[1024];
        while (file.read(buffer, sizeof(buffer))) {
            SocketHandler::send_data(server_socket, std::string(buffer, file.gcount()));
            std::cout << buffer << std::endl;
            memset(buffer, 0, sizeof(buffer)); 
        }
        // Gửi phần còn lại nếu có
        if (file.gcount() > 0) {
            SocketHandler::send_data(server_socket, std::string(buffer, file.gcount()));
            memset(buffer, 0, sizeof(buffer));
        }
        file.close();
        SocketHandler::send_data(server_socket, "END"); // Kết thúc từng file

        std::cout << "Uploaded: " << relative_path << "\n";
    }
    SocketHandler::send_data(server_socket, "END_FOLDER");

    std::string response_folder = SocketHandler::receive_data(server_socket);
    if (response_folder == "OK")
        std::cout << "Folder uploaded successfully.\n";
}

// Hàm tải thư mục từ server
void handle_download_folder(int server_socket) {
    std::string foldername;
    std::cout << "Enter folder name to download: ";
    std::cin >> foldername;

    // Gửi lệnh download_folder tới server
    SocketHandler::send_data(server_socket, "download_folder " + foldername);
    std::string response = SocketHandler::receive_data(server_socket);
    if (response != "READY") {
        std::cout << "Server not ready for folder download.\n";
        return;
    }

    // Tạo thư mục đích trên client nếu chưa tồn tại
    if (!fs::exists(foldername)) {
        fs::create_directory(foldername);
    }

    while (true) {
        std::string file_info = SocketHandler::receive_data(server_socket);
        if (file_info == "END_FOLDER") break; // Kết thúc thư mục

        if (file_info.substr(0, 4) == "FILE") {
            std::string relative_path = file_info.substr(5);
            fs::path file_path = fs::path(foldername) / relative_path;
            fs::path parent_dir = file_path.parent_path();

            // Tạo các thư mục con nếu chưa tồn tại
            if (!fs::exists(parent_dir)) {
                fs::create_directories(parent_dir);
            }

            std::ofstream file(file_path, std::ios::binary);
            if (!file.is_open()) {
                std::cout << "Failed to create file: " << file_path.string() << "\n";
                // Đọc và bỏ qua dữ liệu của file này
                while (true) {
                    std::string chunk = SocketHandler::receive_data(server_socket);
                    if (chunk == "END") break;
                }
                continue;
            }

            // Nhận dữ liệu file từ server
            while (true) {
                std::string chunk = SocketHandler::receive_data(server_socket);
                if (chunk == "END") break;
                file.write(chunk.c_str(), chunk.size());
            }
            file.close();

            std::cout << "Downloaded: " << relative_path << "\n";
        }
    }
    std::cout << "Folder downloaded successfully.\n";
}

// Hàm tải lên một tệp
void handle_upload(int server_socket) {
    std::string filename;
    std::cout << "Enter filename to upload: ";
    std::cin >> filename;

    if (!fs::exists(filename) || !fs::is_regular_file(filename)) {
        std::cout << "File not found.\n";
        return;
    }

    // Gửi lệnh upload tới server
    SocketHandler::send_data(server_socket, "upload " + filename);
    std::string response = SocketHandler::receive_data(server_socket);
    if (response != "READY") {
        std::cout << "Server not ready for upload.\n";
        return;
    }

    // Gửi dữ liệu file
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cout << "Failed to open file.\n";
        SocketHandler::send_data(server_socket, "END"); // Gửi END để tránh server chờ đợi
        return;
    }

    char buffer[1024];
    while (file.read(buffer, sizeof(buffer))) {        
        if (!SocketHandler::send_data(server_socket, std::string(buffer, file.gcount()))) {
            std::cout << "Failed to send data.\n";
            file.close();
            return;
        }
        memset(buffer, 0, sizeof(buffer)); 
    }

    // Gửi phần còn lại nếu có
    if (file.gcount() > 0) {
        if (!SocketHandler::send_data(server_socket, std::string(buffer, 1024))) {
            std::cout << "Failed to send remaining data.\n";
            file.close();
            return;
        }
    }
    file.close();
    SocketHandler::send_data(server_socket, "END"); // Kết thúc upload

    std::cout << "File uploaded successfully.\n";
}

// Hàm tải xuống một tệp
void handle_download(int server_socket) {
    std::string filename;
    std::cout << "Enter filename to download: ";
    std::cin >> filename;

    // Gửi lệnh download tới server
    SocketHandler::send_data(server_socket, "download " + filename);
    std::string response = SocketHandler::receive_data(server_socket);
    if (response != "READY") {
        //std::cout << "Server not ready for download or file does not exist.\n";
        std::cout << response << std::endl;
        return;
    }

    // Nhận dữ liệu file từ server
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cout << "Unable to create file for download.\n";
        // Đọc và bỏ qua dữ liệu của file này
        while (true) {
            std::string chunk = SocketHandler::receive_data(server_socket);
            if (chunk == "END") break;
        }
        return;
    }

    while (true) {
        std::string chunk = SocketHandler::receive_data(server_socket);
        if (chunk == "END") break;
        file.write(chunk.c_str(), chunk.size());
    }
    file.close();

    std::cout << "File downloaded successfully.\n";
}

// Hàm thiết lập quyền cho tệp/thư mục
void handle_set_permission(int server_socket) {
    std::string filename, new_permission;
    std::cout << "Enter filename to set permission: ";
    std::cin >> filename;
    std::cout << "Enter new permission (e.g., r--, rw-, rwx): ";
    std::cin >> new_permission;

    // Gửi lệnh set_permission tới server
    SocketHandler::send_data(server_socket, "set_permission " + filename + " " + new_permission);
    std::string response = SocketHandler::receive_data(server_socket);
    if (response == "Permission updated.") {
        std::cout << "Permission updated successfully.\n";
    } else {
        std::cout << "Failed to update permission.\n";
    }
}

// Hàm tìm kiếm tệp trên server
void handle_search(int server_socket) {
    std::string keyword;
    std::cout << "Enter keyword to search for files: ";
    std::cin >> keyword;

    // Gửi lệnh search tới server
    SocketHandler::send_data(server_socket, "search " + keyword);
    
    while (true) {
        std::string response = SocketHandler::receive_data(server_socket);
        if (response == "END_RESULTS") break;
        if (response.empty()) break;
        std::cout << response << "\n";
    }

    // Cho phép người dùng chọn tải xuống từ kết quả tìm kiếm
    std::cout << "Enter filename to download from search results or 'cancel': ";
    std::string selected_file;
    std::cin >> selected_file;
    if (selected_file != "cancel") {
        // Gửi lệnh download tới server
        SocketHandler::send_data(server_socket, "download " + selected_file);
        std::string download_response = SocketHandler::receive_data(server_socket);
        if (download_response != "READY") {
            std::cout << "Server not ready for download or file does not exist.\n";
            return;
        }

        // Nhận dữ liệu file từ server
        std::ofstream file(selected_file, std::ios::binary);
        if (!file.is_open()) {
            std::cout << "Unable to create file for download.\n";
            // Đọc và bỏ qua dữ liệu của file này
            while (true) {
                std::string chunk = SocketHandler::receive_data(server_socket);
                if (chunk == "END") break;
            }
            return;
        }

        while (true) {
            std::string chunk = SocketHandler::receive_data(server_socket);
            if (chunk == "END") break;
            file.write(chunk.c_str(), chunk.size());
        }
        file.close();

        std::cout << "File downloaded successfully.\n";
    }
}

// Hàm xử lý các lệnh của người dùng
void handle_commands(int server_socket) {
    while (true) {
        std::cout << "\nEnter command (upload, download, upload_folder, download_folder, rename, delete, set_permission, search, exit): ";
        std::string command;
        std::cin >> command;

        if (command == "upload") {
            handle_upload(server_socket);
        } else if (command == "download") {
            handle_download(server_socket);
        } else if (command == "upload_folder") {
            handle_upload_folder(server_socket);
        } else if (command == "download_folder") {
            handle_download_folder(server_socket);
        } else if (command == "set_permission") {
            handle_set_permission(server_socket);
        } else if (command == "search") {
            handle_search(server_socket);
        } else if (command == "rename") {
            std::string old_filename, new_filename;
            std::cout << "Enter filename to be renamed: ";
            std::cin >> old_filename;

            std::cout << "Enter new filename: ";
            std::cin >> new_filename;

            // Gửi lệnh upload tới server
            SocketHandler::send_data(server_socket, "rename " + old_filename + " " + new_filename);
            std::string response = SocketHandler::receive_data(server_socket);
            if (response == "OK") {
                std::cout << "Rename successful.\n";
            }
        } else if (command == "delete") {
            std::string filename;
            std::cout << "Enter filename to be deleted: ";
            std::cin >> filename;

            // Gửi lệnh upload tới server
            SocketHandler::send_data(server_socket, "delete " + filename);
            std::string response = SocketHandler::receive_data(server_socket);
            if (response == "OK") {
                std::cout << "Delete successful.\n";
            }           
        } else if (command == "exit") {
            SocketHandler::send_data(server_socket, "exit");
            std::cout << "Disconnected from server.\n";
            break;
        } else {
            std::cout << "Unknown command.\n";
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: ./client <server_ip> <port>\n";
        return -1;
    }

    std::string server_ip = argv[1];
    int server_port = std::stoi(argv[2]);

    // Kết nối tới server
    int server_socket = SocketHandler::create_client_socket(server_ip, server_port);
    if (server_socket < 0) {
        std::cerr << "Failed to connect to server.\n";
        return -1;
    }

    std::cout << "Connected to server at " << server_ip << ":" << server_port << "\n";

    // Đăng nhập, đăng ký
    // Client Code
    while (true) {
        std::cout << "Enter command (register/login): ";
        std::string command;
        std::cin >> command;
        std::cin.ignore(); // Xóa ký tự newline sau lệnh

        if (command != "register" && command != "login") {
            std::cout << "Invalid command. Please enter 'register' or 'login'.\n";
            continue;
        }

        // Gửi lệnh tới server
        SocketHandler::send_data(server_socket, command);

        if (command == "register") {
            std::string username, password;
            std::cout << "Enter username: ";
            std::getline(std::cin, username);  // Sử dụng getline để nhận thông tin đầy đủ
            SocketHandler::send_data(server_socket, username);

            std::cout << "Enter password: ";
            std::getline(std::cin, password);
            SocketHandler::send_data(server_socket, password);

            std::string response = SocketHandler::receive_data(server_socket);
            std::cout << "Server response: " << response << "\n";
            if (response == "Registration successful.") {
                std::cout << "Registration successful.\n";
                continue;  // Nếu đăng ký thành công, tiếp tục
            } else {
                std::cout << "Registration failed.\n";
                continue;  // Nếu đăng ký thất bại, yêu cầu thử lại
            }
        }

        if (command == "login") {
            std::string username, password;
            std::cout << "Enter username: ";
            std::getline(std::cin, username);
            SocketHandler::send_data(server_socket, username);

            std::cout << "Enter password: ";
            std::getline(std::cin, password);
            SocketHandler::send_data(server_socket, password);

            std::string response = SocketHandler::receive_data(server_socket);
            std::cout << "Server response: " << response << "\n";
                             
            if (response == "Authentication successful.") {
                handle_commands(server_socket);  // Xử lý các lệnh nếu đăng nhập thành công
                break;
            } else {
                std::cout << "Authentication failed. Please try again.\n";
                continue;  // Nếu đăng nhập thất bại, yêu cầu thử lại
            }
        }
    }

    close(server_socket);
    return 0;
}