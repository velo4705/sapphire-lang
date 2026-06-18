#pragma once
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

int      fs_exists(const char* path);
int      fs_is_file(const char* path);
int      fs_is_dir(const char* path);
int      fs_create_dir(const char* path);
char*    fs_read_file(const char* path);
int      fs_write_file(const char* path, const char* content);
int      fs_delete(const char* path);
char*    fs_abs_path(const char* path);
void     fs_free_string(char* str);

#ifdef __cplusplus
}
#endif
