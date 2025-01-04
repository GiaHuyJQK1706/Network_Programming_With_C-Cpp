#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <string>
#include <vector>

class FileManager {
public:
    static bool create_file(const std::string& path, const std::string& content = "");
    static bool create_directory(const std::string& path);
    static bool delete_file(const std::string& path);
    static std::vector<std::string> search_files(const std::string& root, const std::string& keyword);
};

#endif