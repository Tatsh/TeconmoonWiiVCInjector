#ifndef METADATA_H
#define METADATA_H

#include "wiivc_injector.h"

/**
 * Generate app.xml file
 */
bool generate_app_xml(const char *output_path, const game_info_t *info);

/**
 * Generate meta.xml file
 */
bool generate_meta_xml(const char *output_path, const game_info_t *info, const char *title_line1);

#endif /* METADATA_H */
