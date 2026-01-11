// SPDX-License-Identifier: MIT
#include "wiivc/fileformat.h"
#include <algorithm>
#include <cstring>

namespace wiivc {

    Result<FileType> FileFormatDetector::detectFileType(const std::filesystem::path &path) {
        std::ifstream file(path, std::ios::binary);
        if (!file) {
            return std::unexpected(ErrorCode::FileNotFound);
        }

        // Read first 4 bytes to check magic
        std::array<char, 4> magic{};
        file.read(magic.data(), 4);
        if (!file) {
            return std::unexpected(ErrorCode::IOError);
        }

        std::string magicStr(magic.data(), 4);

        // Check for WBFS
        if (magicStr == "WBFS") {
            return FileType::WBFS;
        }

        // Check for NKIT marker (at offset 0x200)
        file.seekg(0x200);
        std::array<char, 4> nkitMagic{};
        file.read(nkitMagic.data(), 4);
        if (file && std::string(nkitMagic.data(), 4) == "NKIT") {
            return FileType::NKIT;
        }

        // Check for NASOS format
        if (magicStr == "WII5" || magicStr == "WII9") {
            return FileType::NASOS;
        }

        // Check for DOL file (power PC executable)
        file.seekg(0);
        uint32_t dolMagic = 0;
        file.read(reinterpret_cast<char *>(&dolMagic), 4);
        if (file && dolMagic == 0x00000100) {
            return FileType::DOL;
        }

        // Check file extension for GameCube
        auto ext = path.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        if (ext == ".gcm") {
            return FileType::GCM;
        }

        // Default to ISO
        if (ext == ".iso") {
            return FileType::ISO;
        }

        return FileType::Unknown;
    }

    Result<bool> FileFormatDetector::isWiiImage(const std::filesystem::path &path) {
        auto typeResult = readGameType(path);
        if (!typeResult) {
            return std::unexpected(typeResult.error());
        }

        // Wii magic number (from C# code: GameType == 2745048157)
        constexpr uint64_t WII_GAME_TYPE = 0xA3C15D1C; // In big-endian
        return *typeResult == WII_GAME_TYPE;
    }

    Result<bool> FileFormatDetector::isGameCubeImage(const std::filesystem::path &path) {
        auto typeResult = readGameType(path);
        if (!typeResult) {
            return std::unexpected(typeResult.error());
        }

        // GameCube magic number (from C# code: GameType == 4440324665927270400)
        constexpr uint64_t GC_GAME_TYPE = 0x3D9F33C2; // Simplified check
        return (*typeResult & 0xFFFFFFFF) == GC_GAME_TYPE;
    }

    Result<std::array<char, 4>> FileFormatDetector::readGameId(const std::filesystem::path &path) {
        std::ifstream file(path, std::ios::binary);
        if (!file) {
            return std::unexpected(ErrorCode::FileNotFound);
        }

        size_t offset = 0;
        auto fileType = detectFileType(path);

        if (fileType && *fileType == FileType::WBFS) {
            offset = 0x200; // WBFS stores game data at 0x200
        } else if (fileType && *fileType == FileType::NASOS) {
            // NASOS has different offsets based on WII5 or WII9
            std::array<char, 4> magic{};
            file.read(magic.data(), 4);
            std::string magicStr(magic.data(), 4);
            if (magicStr == "WII5") {
                offset = 0x1182800;
            } else if (magicStr == "WII9") {
                offset = 0x1FB5000;
            }
        }

        file.seekg(offset);
        std::array<char, 4> id{};
        file.read(id.data(), 4);

        if (!file) {
            return std::unexpected(ErrorCode::IOError);
        }

        return id;
    }

    Result<std::string> FileFormatDetector::readGameName(const std::filesystem::path &path) {
        std::ifstream file(path, std::ios::binary);
        if (!file) {
            return std::unexpected(ErrorCode::FileNotFound);
        }

        size_t offset = 0x20;
        auto fileType = detectFileType(path);

        if (fileType && *fileType == FileType::WBFS) {
            offset = 0x220;
        } else if (fileType && *fileType == FileType::NASOS) {
            std::array<char, 4> magic{};
            file.read(magic.data(), 4);
            std::string magicStr(magic.data(), 4);
            if (magicStr == "WII5") {
                offset = 0x1182800 + 0x20;
            } else if (magicStr == "WII9") {
                offset = 0x1FB5000 + 0x20;
            }
        }

        file.seekg(offset);
        std::string name;
        char c = 0;
        // Read until null terminator or max length
        for (int i = 0; i < 64 && file.get(c) && c != '\0'; ++i) {
            name.push_back(c);
        }

        return name;
    }

    Result<uint64_t> FileFormatDetector::readGameType(const std::filesystem::path &path) {
        std::ifstream file(path, std::ios::binary);
        if (!file) {
            return std::unexpected(ErrorCode::FileNotFound);
        }

        size_t offset = 0x18;
        auto fileType = detectFileType(path);

        if (fileType && *fileType == FileType::WBFS) {
            offset = 0x218;
        } else if (fileType && *fileType == FileType::NASOS) {
            std::array<char, 4> magic{};
            file.read(magic.data(), 4);
            std::string magicStr(magic.data(), 4);
            if (magicStr == "WII5") {
                offset = 0x1182800 + 0x18;
            } else if (magicStr == "WII9") {
                offset = 0x1FB5000 + 0x18;
            }
        }

        file.seekg(offset);
        uint64_t gameType = 0;
        file.read(reinterpret_cast<char *>(&gameType), sizeof(gameType));

        if (!file) {
            return std::unexpected(ErrorCode::IOError);
        }

        return gameType;
    }

} // namespace wiivc
