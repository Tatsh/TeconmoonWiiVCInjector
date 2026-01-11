#include "metadata.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>

bool generate_app_xml(const char *output_path, const game_info_t *info) {
    char dir_path[MAX_PATH_LEN];
    strncpy(dir_path, output_path, MAX_PATH_LEN - 1);

    /* Extract directory path */
    char *last_sep = strrchr(dir_path, '/');
    if (!last_sep) {
        last_sep = strrchr(dir_path, '\\');
    }
    if (last_sep) {
        *last_sep = '\0';
        if (!create_directory(dir_path)) {
            return false;
        }
    }

    FILE *fp = fopen(output_path, "w");
    if (!fp) {
        print_error("Failed to create app.xml: %s\n", output_path);
        return false;
    }

    /* Generate packed title ID (00050002 + hex title ID) */
    char packed_title_id[17];
    snprintf(packed_title_id, sizeof(packed_title_id), "00050002%s", info->title_id_hex);

    fprintf(fp, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
    fprintf(fp, "<app type=\"complex\" access=\"777\">\n");
    fprintf(fp, "  <version type=\"unsignedInt\" length=\"4\">16</version>\n");
    fprintf(fp, "  <os_version type=\"hexBinary\" length=\"8\">000500101000400A</os_version>\n");
    fprintf(fp, "  <title_id type=\"hexBinary\" length=\"8\">%s</title_id>\n", packed_title_id);
    fprintf(fp, "  <title_version type=\"hexBinary\" length=\"2\">0000</title_version>\n");
    fprintf(fp, "  <sdk_version type=\"unsignedInt\" length=\"4\">21204</sdk_version>\n");
    fprintf(fp, "  <app_type type=\"hexBinary\" length=\"4\">8000002E</app_type>\n");
    fprintf(fp, "  <group_id type=\"hexBinary\" length=\"4\">%s</group_id>\n", info->title_id_hex);
    fprintf(
        fp,
        "  <os_mask type=\"hexBinary\" "
        "length=\"32\">0000000000000000000000000000000000000000000000000000000000000000</"
        "os_mask>\n");
    fprintf(fp, "  <common_id type=\"hexBinary\" length=\"8\">0000000000000000</common_id>\n");
    fprintf(fp, "</app>\n");

    fclose(fp);
    return true;
}

bool generate_meta_xml(const char *output_path, const game_info_t *info, const char *title_line1) {
    char dir_path[MAX_PATH_LEN];
    strncpy(dir_path, output_path, MAX_PATH_LEN - 1);

    /* Extract directory path */
    char *last_sep = strrchr(dir_path, '/');
    if (!last_sep) {
        last_sep = strrchr(dir_path, '\\');
    }
    if (last_sep) {
        *last_sep = '\0';
        if (!create_directory(dir_path)) {
            return false;
        }
    }

    FILE *fp = fopen(output_path, "w");
    if (!fp) {
        print_error("Failed to create meta.xml: %s\n", output_path);
        return false;
    }

    /* Generate packed title ID */
    char packed_title_id[17];
    snprintf(packed_title_id, sizeof(packed_title_id), "00050002%s", info->title_id_hex);

    fprintf(fp, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
    fprintf(fp, "<menu type=\"complex\" access=\"777\">\n");
    fprintf(fp, "  <version type=\"unsignedInt\" length=\"4\">33</version>\n");
    fprintf(fp,
            "  <product_code type=\"string\" length=\"32\">WUP-N-%s</product_code>\n",
            info->title_id_text);
    fprintf(fp, "  <content_platform type=\"string\" length=\"32\">WUP</content_platform>\n");
    fprintf(fp, "  <company_code type=\"string\" length=\"8\">0001</company_code>\n");
    fprintf(fp, "  <mastering_date type=\"string\" length=\"32\"></mastering_date>\n");
    fprintf(fp, "  <logo_type type=\"unsignedInt\" length=\"4\">0</logo_type>\n");
    fprintf(fp, "  <app_launch_type type=\"hexBinary\" length=\"4\">00000000</app_launch_type>\n");
    fprintf(fp, "  <invisible_flag type=\"hexBinary\" length=\"4\">00000000</invisible_flag>\n");
    fprintf(fp, "  <no_managed_flag type=\"hexBinary\" length=\"4\">00000000</no_managed_flag>\n");
    fprintf(fp, "  <no_event_log type=\"hexBinary\" length=\"4\">00000002</no_event_log>\n");
    fprintf(fp, "  <no_icon_database type=\"hexBinary\" length=\"4\">00000000</no_icon_database>\n");
    fprintf(fp, "  <launching_flag type=\"hexBinary\" length=\"4\">00000004</launching_flag>\n");
    fprintf(fp, "  <install_flag type=\"hexBinary\" length=\"4\">00000000</install_flag>\n");
    fprintf(fp, "  <closing_msg type=\"unsignedInt\" length=\"4\">0</closing_msg>\n");
    fprintf(fp, "  <title_version type=\"unsignedInt\" length=\"4\">0</title_version>\n");
    fprintf(fp, "  <title_id type=\"hexBinary\" length=\"8\">%s</title_id>\n", packed_title_id);
    fprintf(fp, "  <group_id type=\"hexBinary\" length=\"4\">%s</group_id>\n", info->title_id_hex);
    fprintf(fp, "  <boss_id type=\"hexBinary\" length=\"8\">0000000000000000</boss_id>\n");
    fprintf(fp, "  <os_version type=\"hexBinary\" length=\"8\">000500101000400A</os_version>\n");
    fprintf(fp, "  <app_size type=\"hexBinary\" length=\"8\">0000000000000000</app_size>\n");
    fprintf(
        fp, "  <common_save_size type=\"hexBinary\" length=\"8\">0000000000000000</common_save_size>\n");
    fprintf(
        fp, "  <account_save_size type=\"hexBinary\" length=\"8\">0000000000000000</account_save_size>\n");
    fprintf(
        fp, "  <common_boss_size type=\"hexBinary\" length=\"8\">0000000000000000</common_boss_size>\n");
    fprintf(
        fp, "  <account_boss_size type=\"hexBinary\" length=\"8\">0000000000000000</account_boss_size>\n");
    fprintf(fp, "  <save_no_rollback type=\"unsignedInt\" length=\"4\">0</save_no_rollback>\n");
    fprintf(fp, "  <join_game_id type=\"hexBinary\" length=\"4\">00000000</join_game_id>\n");
    fprintf(
        fp,
        "  <join_game_mode_mask type=\"hexBinary\" "
        "length=\"8\">0000000000000000</join_game_mode_mask>\n");
    fprintf(fp, "  <bg_daemon_enable type=\"unsignedInt\" length=\"4\">0</bg_daemon_enable>\n");
    fprintf(fp, "  <olv_accesskey type=\"unsignedInt\" length=\"4\">3921400692</olv_accesskey>\n");
    fprintf(fp, "  <wood_tin type=\"unsignedInt\" length=\"4\">0</wood_tin>\n");
    fprintf(fp, "  <e_manual type=\"unsignedInt\" length=\"4\">0</e_manual>\n");
    fprintf(fp, "  <e_manual_version type=\"unsignedInt\" length=\"4\">0</e_manual_version>\n");
    fprintf(fp, "  <region type=\"hexBinary\" length=\"4\">00000002</region>\n");
    fprintf(fp, "  <drc_use type=\"unsignedInt\" length=\"4\">1</drc_use>\n");

    /* Add title in all languages */
    const char *languages[] = {"ja", "en", "fr", "de", "it",  "es",
                               "zhs", "ko", "nl", "pt", "ru", "zht"};
    for (int i = 0; i < 12; i++) {
        fprintf(fp,
                "  <longname_%s type=\"string\" length=\"512\">%s</longname_%s>\n",
                languages[i],
                title_line1,
                languages[i]);
    }
    for (int i = 0; i < 12; i++) {
        fprintf(fp,
                "  <shortname_%s type=\"string\" length=\"512\">%s</shortname_%s>\n",
                languages[i],
                title_line1,
                languages[i]);
    }

    fprintf(fp, "</menu>\n");

    fclose(fp);
    return true;
}
