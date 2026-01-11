#ifndef ISO_READER_H
#define ISO_READER_H

#include "wiivc_injector.h"
#include <stdbool.h>
#include <stdint.h>

/**
 * Read game information from an ISO file
 */
bool read_game_info(const char *filepath, game_info_t *info);

/**
 * Detect the format of the game file
 */
format_type_t detect_format(FILE *fp);

/**
 * Check if file is WBFS format
 */
bool is_wbfs(FILE *fp);

/**
 * Check if file is NKIT format
 */
bool is_nkit(FILE *fp);

/**
 * Check if file is NASOS format
 */
bool is_nasos(FILE *fp);

#endif /* ISO_READER_H */
