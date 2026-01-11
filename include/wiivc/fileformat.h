// SPDX-License-Identifier: MIT
#pragma once

#include "types.h"
#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string_view>

namespace wiivc {

    class FileFormatDetector {
      public:
        // Detect file type from file header
        [[nodiscard]] static Result<FileType> detectFileType(const std::filesystem::path &path);

        // Check if file is a valid Wii disc image
        [[nodiscard]] static Result<bool> isWiiImage(const std::filesystem::path &path);

        // Check if file is a valid GameCube disc image
        [[nodiscard]] static Result<bool> isGameCubeImage(const std::filesystem::path &path);

        // Read game ID from disc image
        [[nodiscard]] static Result<std::array<char, 4>> readGameId(const std::filesystem::path &path);

        // Read internal game name from disc image
        [[nodiscard]] static Result<std::string> readGameName(const std::filesystem::path &path);

        // Read game type identifier
        [[nodiscard]] static Result<uint64_t> readGameType(const std::filesystem::path &path);

      private:
        static constexpr uint64_t WII_MAGIC = 0x5D1C9EA3; // Wii disc magic in header
        static constexpr uint64_t GC_MAGIC = 0xC2339F3D; // GameCube disc magic
    };

    // Helper to read bytes from file at offset
    template<typename T>
    [[nodiscard]] inline Result<T> readAtOffset(const std::filesystem::path &path, size_t offset) {
        std::ifstream file(path, std::ios::binary);
        if (!file) {
            return std::unexpected(ErrorCode::FileNotFound);
        }

        file.seekg(offset);
        if (!file) {
            return std::unexpected(ErrorCode::IOError);
        }

        T value;
        file.read(reinterpret_cast<char *>(&value), sizeof(T));
        if (!file) {
            return std::unexpected(ErrorCode::IOError);
        }

        return value;
    }

} // namespace wiivc
