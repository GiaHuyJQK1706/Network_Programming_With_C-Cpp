#include "file_manager.h"
#include <filesystem>
#include <fstream>
#include <iostream>

bool FileManager::create_file(const std::string& path, const std::string& content) {
    std::ofstream file(path);
    if (!file.is_open()) return false;
    file << content;
    file.close();
    return true;
}

bool FileManager::create_directory(const std::string& path) {
    return std::filesystem::create_directory(path);
}

bool FileManager::delete_file(const std::string& path) {
    return std::filesystem::remove(path);
}

std::vector<std::string> FileManager::search_files(const std::string& root, const std::string& keyword) {
    std::vector<std::string> results;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(root)) {
        if (entry.path().filename().string().find(keyword) != std::string::npos) {
            results.push_back(entry.path().string());
        }
    }
    return results;
}