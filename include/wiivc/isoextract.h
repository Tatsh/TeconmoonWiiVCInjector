// SPDX-License-Identifier: MIT
#pragma once

#include "types.h"
#include <cstdint>
#include <filesystem>
#include <vector>

namespace wiivc::isoextract {

    // Basic ISO/WBFS extraction without external tools
    class IsoExtractor {
      public:
        // Extract ISO contents to directory
        [[nodiscard]] Result<void> extractIso(const std::filesystem::path &isoPath,
                                               const std::filesystem::path &outputDir);

        // Trim ISO by removing padding
        [[nodiscard]] Result<void> trimIso(const std::filesystem::path &isoPath,
                                            const std::filesystem::path &outputPath);

        // Convert between formats (basic support)
        [[nodiscard]] Result<void> convertFormat(const std::filesystem::path &inputPath,
                                                  const std::filesystem::path &outputPath);

        // Get disc information
        struct DiscInfo {
            std::array<char, 6> gameId{};
            std::string gameName;
            uint64_t size{0};
        };

        [[nodiscard]] Result<DiscInfo> getDiscInfo(const std::filesystem::path &discPath);

      private:
        [[nodiscard]] Result<void> extractWiiDisc(const std::filesystem::path &isoPath,
                                                   const std::filesystem::path &outputDir);
    };

} // namespace wiivc::isoextract
