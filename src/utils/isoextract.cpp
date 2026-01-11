// SPDX-License-Identifier: MIT
#include "wiivc/isoextract.h"
#include "wiivc/fileformat.h"
#include <cstring>
#include <fstream>

namespace wiivc::isoextract {

    Result<IsoExtractor::DiscInfo> IsoExtractor::getDiscInfo(const std::filesystem::path &discPath) {
        // Use existing file format detector
        auto idResult = FileFormatDetector::readGameId(discPath);
        if (!idResult) {
            return std::unexpected(idResult.error());
        }

        auto nameResult = FileFormatDetector::readGameName(discPath);
        if (!nameResult) {
            return std::unexpected(nameResult.error());
        }

        std::ifstream file(discPath, std::ios::binary);
        if (!file) {
            return std::unexpected(ErrorCode::FileNotFound);
        }

        file.seekg(0, std::ios::end);
        uint64_t size = file.tellg();

        DiscInfo info;
        std::memcpy(info.gameId.data(), idResult->data(), std::min(idResult->size(), info.gameId.size()));
        info.gameName = *nameResult;
        info.size = size;

        return info;
    }

    Result<void> IsoExtractor::extractWiiDisc(const std::filesystem::path &isoPath,
                                               const std::filesystem::path &outputDir) {
        std::ifstream iso(isoPath, std::ios::binary);
        if (!iso) {
            return std::unexpected(ErrorCode::FileNotFound);
        }

        // Create output directory
        std::filesystem::create_directories(outputDir);

        // Simplified extraction - copy ISO data
        // Full implementation would parse Wii disc filesystem
        auto dataDir = outputDir / "DATA";
        std::filesystem::create_directories(dataDir);

        auto outputFile = dataDir / "game.iso";
        std::ofstream output(outputFile, std::ios::binary);
        if (!output) {
            return std::unexpected(ErrorCode::IOError);
        }

        // Simple copy for now
        output << iso.rdbuf();

        return {};
    }

    Result<void> IsoExtractor::extractIso(const std::filesystem::path &isoPath,
                                           const std::filesystem::path &outputDir) {
        if (!std::filesystem::exists(isoPath)) {
            return std::unexpected(ErrorCode::FileNotFound);
        }

        return extractWiiDisc(isoPath, outputDir);
    }

    Result<void> IsoExtractor::trimIso(const std::filesystem::path &isoPath,
                                        const std::filesystem::path &outputPath) {
        std::ifstream input(isoPath, std::ios::binary);
        if (!input) {
            return std::unexpected(ErrorCode::FileNotFound);
        }

        std::ofstream output(outputPath, std::ios::binary);
        if (!output) {
            return std::unexpected(ErrorCode::IOError);
        }

        // Get file size
        input.seekg(0, std::ios::end);
        size_t fileSize = input.tellg();
        input.seekg(0, std::ios::beg);

        // Find actual data size (simplified - would parse disc structure)
        // For now, copy entire file
        constexpr size_t bufferSize = 1024 * 1024; // 1MB buffer
        std::vector<char> buffer(bufferSize);

        while (input.read(buffer.data(), bufferSize) || input.gcount() > 0) {
            output.write(buffer.data(), input.gcount());
        }

        return {};
    }

    Result<void> IsoExtractor::convertFormat(const std::filesystem::path &inputPath,
                                              const std::filesystem::path &outputPath) {
        // Simplified conversion - just copy
        // Full implementation would handle WBFS, NKIT formats
        std::ifstream input(inputPath, std::ios::binary);
        if (!input) {
            return std::unexpected(ErrorCode::FileNotFound);
        }

        std::ofstream output(outputPath, std::ios::binary);
        if (!output) {
            return std::unexpected(ErrorCode::IOError);
        }

        output << input.rdbuf();

        return {};
    }

} // namespace wiivc::isoextract
