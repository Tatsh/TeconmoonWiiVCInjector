#include "utils.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef PLATFORM_WINDOWS
#include <direct.h>
#include <io.h>
#define mkdir(path, mode) _mkdir(path)
#define access _access
#define F_OK 0
#else
#include <unistd.h>
#endif

bool create_directory(const char *path) {
    char tmp[MAX_PATH_LEN];
    char *p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp), "%s", path);
    len = strlen(tmp);
    if (tmp[len - 1] == '/' || tmp[len - 1] == '\\')
        tmp[len - 1] = 0;

    for (p = tmp + 1; *p; p++) {
        if (*p == '/' || *p == '\\') {
            *p = 0;
            if (mkdir(tmp, 0755) != 0) {
                if (!directory_exists(tmp)) {
                    return false;
                }
            }
            *p = PATH_SEP[0];
        }
    }

    if (mkdir(tmp, 0755) != 0) {
        if (!directory_exists(tmp)) {
            return false;
        }
    }

    return true;
}

bool file_exists(const char *path) {
#ifdef PLATFORM_WINDOWS
    return _access(path, F_OK) == 0;
#else
    return access(path, F_OK) == 0;
#endif
}

bool directory_exists(const char *path) {
    struct stat st;
    if (stat(path, &st) != 0) {
        return false;
    }
    return (st.st_mode & S_IFDIR) != 0;
}

bool copy_file(const char *src, const char *dst) {
    FILE *src_fp = fopen(src, "rb");
    if (!src_fp) {
        return false;
    }

    FILE *dst_fp = fopen(dst, "wb");
    if (!dst_fp) {
        fclose(src_fp);
        return false;
    }

    char buffer[8192];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), src_fp)) > 0) {
        if (fwrite(buffer, 1, bytes, dst_fp) != bytes) {
            fclose(src_fp);
            fclose(dst_fp);
            return false;
        }
    }

    fclose(src_fp);
    fclose(dst_fp);
    return true;
}

const char *get_temp_dir(void) {
#ifdef PLATFORM_WINDOWS
    static char temp_dir[MAX_PATH_LEN];
    if (GetTempPath(MAX_PATH_LEN, temp_dir) != 0) {
        return temp_dir;
    }
    return "C:\\Temp\\";
#else
    const char *tmp = getenv("TMPDIR");
    if (tmp) {
        return tmp;
    }
    return "/tmp/";
#endif
}

void bytes_to_hex(const uint8_t *bytes, size_t len, char *hex_str) {
    for (size_t i = 0; i < len; i++) {
        sprintf(hex_str + (i * 2), "%02X", bytes[i]);
    }
    hex_str[len * 2] = '\0';
}

void print_error(const char *format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(stderr, "Error: ");
    vfprintf(stderr, format, args);
    va_end(args);
}

void print_verbose(bool verbose, const char *format, ...) {
    if (!verbose) {
        return;
    }
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}
