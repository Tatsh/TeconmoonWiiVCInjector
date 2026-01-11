#ifndef WIIVC_INJECTOR_H
#define WIIVC_INJECTOR_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/* Platform detection */
#if defined(_WIN32) || defined(_WIN64)
#define PLATFORM_WINDOWS
#elif defined(__APPLE__) && defined(__MACH__)
#define PLATFORM_MACOS
#elif defined(__linux__)
#define PLATFORM_LINUX
#else
#define PLATFORM_UNIX
#endif

/* Path separator */
#ifdef PLATFORM_WINDOWS
#define PATH_SEP "\\"
#else
#define PATH_SEP "/"
#endif

/* Constants */
#define MAX_PATH_LEN 4096
#define TITLE_ID_LEN 4
#define TITLE_ID_HEX_LEN 8
#define GAME_NAME_MAX 256

/* Enums */
typedef enum {
    SYSTEM_WII,
    SYSTEM_WII_HOMEBREW,
    SYSTEM_WII_NAND,
    SYSTEM_GAMECUBE
} system_type_t;

typedef enum { FORMAT_ISO, FORMAT_WBFS, FORMAT_NKIT, FORMAT_NASOS } format_type_t;

/* Structures */
typedef struct {
    char game_path[MAX_PATH_LEN];
    char icon_path[MAX_PATH_LEN];
    char banner_path[MAX_PATH_LEN];
    char output_path[MAX_PATH_LEN];
    system_type_t system_type;
    bool verbose;
} config_t;

typedef struct {
    uint32_t title_id_int;
    char title_id_hex[TITLE_ID_HEX_LEN + 1];
    char title_id_text[TITLE_ID_LEN + 1];
    char game_name[GAME_NAME_MAX];
    uint64_t game_type;
    format_type_t format;
} game_info_t;

#endif /* WIIVC_INJECTOR_H */
