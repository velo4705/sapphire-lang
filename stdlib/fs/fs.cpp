#include "fs.h"
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <climits>

int fs_exists(const char* path) {
    struct stat buffer;
    return stat(path, &buffer) == 0 ? 1 : 0;
}

int fs_is_file(const char* path) {
    struct stat buffer;
    if (stat(path, &buffer) != 0) return 0;
    return S_ISREG(buffer.st_mode) ? 1 : 0;
}

int fs_is_dir(const char* path) {
    struct stat buffer;
    if (stat(path, &buffer) != 0) return 0;
    return S_ISDIR(buffer.st_mode) ? 1 : 0;
}

int fs_create_dir(const char* path) {
    return mkdir(path, 0755) == 0 ? 1 : 0;
}

char* fs_read_file(const char* path) {
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file.is_open()) return nullptr;

    std::ostringstream ss;
    ss << file.rdbuf();
    std::string content = ss.str();

    char* result = (char*)std::malloc(content.size() + 1);
    if (result) {
        std::memcpy(result, content.c_str(), content.size() + 1);
    }
    return result;
}

int fs_write_file(const char* path, const char* content) {
    std::ofstream file(path, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!file.is_open()) return 0;
    file << content;
    return file.good() ? 1 : 0;
}

int fs_delete(const char* path) {
    return unlink(path) == 0 ? 1 : 0;
}

char* fs_abs_path(const char* path) {
    char resolved[PATH_MAX];
    if (realpath(path, resolved) != nullptr) {
        char* result = (char*)std::malloc(strlen(resolved) + 1);
        if (result) {
            std::strcpy(result, resolved);
        }
        return result;
    }
    return nullptr;
}

void fs_free_string(char* str) {
    if (str) std::free(str);
}
