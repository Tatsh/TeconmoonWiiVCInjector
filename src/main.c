#include "wiivc_injector.h"
#include "iso_reader.h"
#include "metadata.h"
#include "utils.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_usage(const char *prog_name) {
    printf("Usage: %s [OPTIONS]\n", prog_name);
    printf("\nWii Virtual Console Injector for Wii U - CLI version\n");
    printf("\nRequired Options:\n");
    printf("  -g, --game PATH        Path to game ISO/WBFS file\n");
    printf("  -i, --icon PATH        Path to icon image (128x128)\n");
    printf("  -b, --banner PATH      Path to banner image (1280x720)\n");
    printf("  -o, --output PATH      Output directory path\n");
    printf("\nOptional:\n");
    printf("  -t, --type TYPE        System type: wii, gcn, dol, nand (default: wii)\n");
    printf("  -v, --verbose          Enable verbose output\n");
    printf("  -h, --help             Show this help message\n");
    printf("\nExample:\n");
    printf("  %s -g game.iso -i icon.png -b banner.png -o output/\n", prog_name);
}

static bool parse_arguments(int argc, char **argv, config_t *config) {
    int opt;
    static struct option long_options[] = {{"game", required_argument, 0, 'g'},
                                           {"icon", required_argument, 0, 'i'},
                                           {"banner", required_argument, 0, 'b'},
                                           {"output", required_argument, 0, 'o'},
                                           {"type", required_argument, 0, 't'},
                                           {"verbose", no_argument, 0, 'v'},
                                           {"help", no_argument, 0, 'h'},
                                           {0, 0, 0, 0}};

    /* Initialize config with defaults */
    memset(config, 0, sizeof(config_t));
    config->system_type = SYSTEM_WII;
    config->verbose = false;

    while ((opt = getopt_long(argc, argv, "g:i:b:o:t:vh", long_options, NULL)) != -1) {
        switch (opt) {
        case 'g':
            strncpy(config->game_path, optarg, MAX_PATH_LEN - 1);
            break;
        case 'i':
            strncpy(config->icon_path, optarg, MAX_PATH_LEN - 1);
            break;
        case 'b':
            strncpy(config->banner_path, optarg, MAX_PATH_LEN - 1);
            break;
        case 'o':
            strncpy(config->output_path, optarg, MAX_PATH_LEN - 1);
            break;
        case 't':
            if (strcmp(optarg, "wii") == 0) {
                config->system_type = SYSTEM_WII;
            } else if (strcmp(optarg, "gcn") == 0) {
                config->system_type = SYSTEM_GAMECUBE;
            } else if (strcmp(optarg, "dol") == 0) {
                config->system_type = SYSTEM_WII_HOMEBREW;
            } else if (strcmp(optarg, "nand") == 0) {
                config->system_type = SYSTEM_WII_NAND;
            } else {
                print_error("Unknown system type: %s\n", optarg);
                return false;
            }
            break;
        case 'v':
            config->verbose = true;
            break;
        case 'h':
            print_usage(argv[0]);
            exit(0);
        default:
            print_usage(argv[0]);
            return false;
        }
    }

    /* Validate required arguments */
    if (config->game_path[0] == '\0') {
        print_error("Game path is required\n");
        return false;
    }
    if (config->icon_path[0] == '\0') {
        print_error("Icon path is required\n");
        return false;
    }
    if (config->banner_path[0] == '\0') {
        print_error("Banner path is required\n");
        return false;
    }
    if (config->output_path[0] == '\0') {
        print_error("Output path is required\n");
        return false;
    }

    return true;
}

int main(int argc, char **argv) {
    config_t config;
    game_info_t game_info;

    printf("Wii Virtual Console Injector v1.0.0\n");
    printf("=====================================\n\n");

    /* Parse command line arguments */
    if (!parse_arguments(argc, argv, &config)) {
        return 1;
    }

    /* Verify input files exist */
    if (!file_exists(config.game_path)) {
        print_error("Game file not found: %s\n", config.game_path);
        return 1;
    }
    if (!file_exists(config.icon_path)) {
        print_error("Icon file not found: %s\n", config.icon_path);
        return 1;
    }
    if (!file_exists(config.banner_path)) {
        print_error("Banner file not found: %s\n", config.banner_path);
        return 1;
    }

    /* Create output directory */
    if (!create_directory(config.output_path)) {
        print_error("Failed to create output directory: %s\n", config.output_path);
        return 1;
    }

    print_verbose(config.verbose, "Reading game information...\n");

    /* Read game information from ISO */
    if (!read_game_info(config.game_path, &game_info)) {
        print_error("Failed to read game information\n");
        return 1;
    }

    print_verbose(config.verbose, "Game: %s\n", game_info.game_name);
    print_verbose(config.verbose, "Title ID: %s\n", game_info.title_id_hex);

    /* Generate metadata files */
    print_verbose(config.verbose, "Generating metadata files...\n");

    char app_xml_path[MAX_PATH_LEN];
    snprintf(app_xml_path, MAX_PATH_LEN, "%s%scode%sapp.xml", config.output_path, PATH_SEP, PATH_SEP);

    if (!generate_app_xml(app_xml_path, &game_info)) {
        print_error("Failed to generate app.xml\n");
        return 1;
    }

    char meta_xml_path[MAX_PATH_LEN];
    snprintf(
        meta_xml_path, MAX_PATH_LEN, "%s%smeta%smeta.xml", config.output_path, PATH_SEP, PATH_SEP);

    if (!generate_meta_xml(meta_xml_path, &game_info, game_info.game_name)) {
        print_error("Failed to generate meta.xml\n");
        return 1;
    }

    printf("\nConversion completed successfully!\n");
    printf("Output directory: %s\n", config.output_path);

    printf("\nNOTE: This is a simplified CLI implementation.\n");
    printf("The following steps still need external tools:\n");
    printf("  - Image conversion (PNG to TGA)\n");
    printf("  - ISO/WBFS conversion\n");
    printf("  - NFS format conversion\n");
    printf("  - Content encryption with NUSPacker\n");

    return 0;
}
