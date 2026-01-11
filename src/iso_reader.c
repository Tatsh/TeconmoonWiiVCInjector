#include "iso_reader.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>

#define WBFS_MAGIC 0x53464257 /* "WBFS" in little endian */
#define NKIT_OFFSET 0x200
#define WII_MAGIC 0x5D1C9EA3 /* Wii disc magic */
#define GC_MAGIC 0xC2339F3D  /* GameCube disc magic (part of it) */

bool is_wbfs(FILE *fp) {
    uint32_t magic;
    fseek(fp, 0, SEEK_SET);
    if (fread(&magic, sizeof(uint32_t), 1, fp) != 1) {
        return false;
    }
    return magic == WBFS_MAGIC;
}

bool is_nkit(FILE *fp) {
    char magic[4];
    fseek(fp, NKIT_OFFSET, SEEK_SET);
    if (fread(magic, 1, 4, fp) != 4) {
        return false;
    }
    return memcmp(magic, "NKIT", 4) == 0;
}

bool is_nasos(FILE *fp) {
    char magic[4];
    fseek(fp, 0, SEEK_SET);
    if (fread(magic, 1, 4, fp) != 4) {
        return false;
    }
    return memcmp(magic, "WII5", 4) == 0 || memcmp(magic, "WII9", 4) == 0;
}

format_type_t detect_format(FILE *fp) {
    if (is_wbfs(fp)) {
        return FORMAT_WBFS;
    }
    if (is_nasos(fp)) {
        return FORMAT_NASOS;
    }
    if (is_nkit(fp)) {
        return FORMAT_NKIT;
    }
    return FORMAT_ISO;
}

bool read_game_info(const char *filepath, game_info_t *info) {
    FILE *fp = fopen(filepath, "rb");
    if (!fp) {
        print_error("Failed to open file: %s\n", filepath);
        return false;
    }

    memset(info, 0, sizeof(game_info_t));

    /* Detect format */
    info->format = detect_format(fp);

    /* Determine offset based on format */
    long offset = 0;
    if (info->format == FORMAT_WBFS) {
        offset = 0x200;
    } else if (info->format == FORMAT_NASOS) {
        char magic[4];
        fseek(fp, 0, SEEK_SET);
        fread(magic, 1, 4, fp);
        if (memcmp(magic, "WII5", 4) == 0) {
            offset = 0x1182800;
        } else if (memcmp(magic, "WII9", 4) == 0) {
            offset = 0x1FB5000;
        }
    }

    /* Read title ID (4 bytes at offset 0x00) */
    fseek(fp, offset, SEEK_SET);
    if (fread(&info->title_id_int, sizeof(uint32_t), 1, fp) != 1) {
        print_error("Failed to read title ID\n");
        fclose(fp);
        return false;
    }

    /* Convert to hex string */
    bytes_to_hex((uint8_t *)&info->title_id_int, 4, info->title_id_hex);

    /* Read game type (8 bytes at offset 0x18) */
    fseek(fp, offset + 0x18, SEEK_SET);
    if (fread(&info->game_type, sizeof(uint64_t), 1, fp) != 1) {
        print_error("Failed to read game type\n");
        fclose(fp);
        return false;
    }

    /* Read game name (at offset 0x20, null-terminated) */
    fseek(fp, offset + 0x20, SEEK_SET);
    size_t i = 0;
    char c;
    while (i < GAME_NAME_MAX - 1 && fread(&c, 1, 1, fp) == 1 && c != '\0') {
        info->game_name[i++] = c;
    }
    info->game_name[i] = '\0';

    /* Extract text title ID from hex */
    for (i = 0; i < 4 && i < TITLE_ID_LEN; i++) {
        info->title_id_text[i] = (char)((uint8_t *)&info->title_id_int)[i];
    }
    info->title_id_text[TITLE_ID_LEN] = '\0';

    fclose(fp);
    return true;
}
