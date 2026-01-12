// SPDX-License-Identifier: MIT
#include "wiivc/isotools.h"
#include "wiivc/isoextract.h"
#include <spdlog/spdlog.h>

namespace wiivc::isotools {

    Result<bool> WitTool::isAvailable() {
        // Native library is always available
        return true;
    }

    Result<void> WitTool::extractISO(const std::filesystem::path &isoPath,
                                      const std::filesystem::path &outputDir,
                                      bool verbose) {
        if (!std::filesystem::exists(isoPath)) {
            return std::unexpected(ErrorCode::FileNotFound);
        }

        // Use ISO extractor library
        isoextract::IsoExtractor extractor;
        auto result = extractor.extractIso(isoPath, outputDir);

        if (verbose && result) {
            spdlog::info("ISO extracted: {} -> {}", isoPath.string(), outputDir.string());
        }

        return result;
    }

    Result<void> WitTool::convertFormat(const std::filesystem::path &inputPath,
                                         const std::filesystem::path &outputPath,
                                         std::string_view targetFormat,
                                         bool verbose) {
        if (!std::filesystem::exists(inputPath)) {
            return std::unexpected(ErrorCode::FileNotFound);
        }

        // Use ISO extractor library
        isoextract::IsoExtractor extractor;
        auto result = extractor.convertFormat(inputPath, outputPath);

        if (verbose && result) {
            spdlog::info("Format converted: {} -> {}", inputPath.string(), outputPath.string());
        }

        return result;
    }

    Result<WitTool::DiscInfo> WitTool::getDiscInfo(const std::filesystem::path &discPath) {
        if (!std::filesystem::exists(discPath)) {
            return std::unexpected(ErrorCode::FileNotFound);
        }

        // Use ISO extractor library
        isoextract::IsoExtractor extractor;
        auto extractorInfo = extractor.getDiscInfo(discPath);
        if (!extractorInfo) {
            return std::unexpected(extractorInfo.error());
        }

        DiscInfo info;
        info.gameId = std::string(extractorInfo->gameId.data(), extractorInfo->gameId.size());
        info.gameName = extractorInfo->gameName;
        info.size = extractorInfo->size;
        info.format = "ISO"; // Simplified

        return info;
    }

    Result<void> WitTool::trimISO(const std::filesystem::path &isoPath,
                                   const std::filesystem::path &outputPath,
                                   bool verbose) {
        if (!std::filesystem::exists(isoPath)) {
            return std::unexpected(ErrorCode::FileNotFound);
        }

        // Use ISO extractor library
        isoextract::IsoExtractor extractor;
        auto result = extractor.trimIso(isoPath, outputPath);

        if (verbose && result) {
            spdlog::info("ISO trimmed: {} -> {}", isoPath.string(), outputPath.string());
        }

        return result;
    }

} // namespace wiivc::isotools
