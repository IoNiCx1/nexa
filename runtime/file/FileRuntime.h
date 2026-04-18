#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Read entire file → heap string (caller does not free)
char* nexa_file_read(const char* path);

// Overwrite file with content
void nexa_file_write(const char* path, const char* content);

// Append content to file
void nexa_file_append(const char* path, const char* content);

// Returns 1 if file exists, 0 otherwise
int nexa_file_exists(const char* path);

#ifdef __cplusplus
}
#endif