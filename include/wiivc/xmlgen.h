// SPDX-License-Identifier: MIT
#pragma once

#include "types.h"
#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>

namespace wiivc::xmlgen {

    // Configuration for app.xml generation
    struct AppXMLConfig {
        std::string titleId;     // 8-char hex title ID (e.g., "00050002")
        std::string titleIdHex;  // Title ID in hex
        uint32_t sdkVersion{21204};
        std::string osVersion{"000500101000400A"};
    };

    // Configuration for meta.xml generation
    struct MetaXMLConfig {
        std::string titleId;           // 8-char hex title ID
        std::string titleIdHex;        // Title ID in hex
        std::string productCode;       // WUP-N-XXXX
        std::string longName;          // Game long name
        std::string shortName;         // Game short name
        std::string longNameLine2;     // Optional second line for long name
        uint32_t drcUse{1};            // DRC usage flag
        bool enableLine2{false};       // Enable second line for long name
    };

    // Generate app.xml content
    [[nodiscard]] Result<std::string> generateAppXML(const AppXMLConfig &config);

    // Generate meta.xml content
    [[nodiscard]] Result<std::string> generateMetaXML(const MetaXMLConfig &config);

    // Save app.xml to file
    [[nodiscard]] Result<void> saveAppXML(const std::filesystem::path &path,
                                           const AppXMLConfig &config);

    // Save meta.xml to file
    [[nodiscard]] Result<void> saveMetaXML(const std::filesystem::path &path,
                                            const MetaXMLConfig &config);

} // namespace wiivc::xmlgen
