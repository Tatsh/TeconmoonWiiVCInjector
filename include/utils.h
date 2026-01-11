#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>

/**
 * Cross-platform utilities
 */

/**
 * Create a directory (including parent directories if needed)
 */
bool create_directory(const char *path);

/**
 * Check if file exists
 */
bool file_exists(const char *path);

/**
 * Check if directory exists
 */
bool directory_exists(const char *path);

/**
 * Copy a file
 */
bool copy_file(const char *src, const char *dst);

/**
 * Get temporary directory path
 */
const char *get_temp_dir(void);

/**
 * Convert bytes to hex string
 */
void bytes_to_hex(const uint8_t *bytes, size_t len, char *hex_str);

/**
 * Print error message
 */
void print_error(const char *format, ...);

/**
 * Print verbose message
 */
void print_verbose(bool verbose, const char *format, ...);

#endif /* UTILS_H */
