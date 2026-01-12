// SPDX-License-Identifier: MIT
#pragma once

#include "types.h"
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace wiivc::isotools {

    // Native ISO manipulation operations
    class WitTool {
      public:
        // Check if library is available
        [[nodiscard]] Result<bool> isAvailable();

        // Extract ISO to directory
        [[nodiscard]] Result<void> extractISO(const std::filesystem::path &isoPath,
                                               const std::filesystem::path &outputDir,
                                               bool verbose = false);

        // Convert ISO format (e.g., WBFS to ISO)
        [[nodiscard]] Result<void> convertFormat(const std::filesystem::path &inputPath,
                                                  const std::filesystem::path &outputPath,
                                                  std::string_view targetFormat,
                                                  bool verbose = false);

        // Get disc information
        struct DiscInfo {
            std::string gameId;
            std::string gameName;
            std::string region;
            uint64_t size{0};
            std::string format;
        };

        [[nodiscard]] Result<DiscInfo> getDiscInfo(const std::filesystem::path &discPath);

        // Trim/scrub ISO to remove padding
        [[nodiscard]] Result<void> trimISO(const std::filesystem::path &isoPath,
                                            const std::filesystem::path &outputPath,
                                            bool verbose = false);
    };

} // namespace wiivc::isotools
