// SPDX-License-Identifier: MIT
#include "wiivc/fileformat.h"
#include <algorithm>
#include <bit>
#include <cstring>

namespace wiivc {

    namespace {
        // Disc image format offsets
        constexpr size_t WBFS_GAME_DATA_OFFSET = 0x200;
        constexpr size_t NASOS_WII5_OFFSET = 0x1182800;
        constexpr size_t NASOS_WII9_OFFSET = 0x1FB5000;
        constexpr size_t GAME_NAME_OFFSET = 0x20;
        constexpr size_t GAME_TYPE_OFFSET = 0x18;

        // Read 32-bit big-endian value
        uint32_t readBE32(std::ifstream &file) {
            std::array<uint8_t, 4> bytes{};
            file.read(reinterpret_cast<char *>(bytes.data()), 4);
            if constexpr (std::endian::native == std::endian::little) {
                return (static_cast<uint32_t>(bytes[0]) << 24) |
                       (static_cast<uint32_t>(bytes[1]) << 16) |
                       (static_cast<uint32_t>(bytes[2]) << 8) | static_cast<uint32_t>(bytes[3]);
            } else {
                uint32_t value;
                std::memcpy(&value, bytes.data(), 4);
                return value;
            }
        }
    } // namespace

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

        // Check for DOL file (PowerPC executable)
        file.seekg(0);
        uint32_t dolMagic = readBE32(file);
        // DOL files start with 0x00000100 in big-endian
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
            offset = WBFS_GAME_DATA_OFFSET;
        } else if (fileType && *fileType == FileType::NASOS) {
            // NASOS has different offsets based on WII5 or WII9
            std::array<char, 4> magic{};
            file.read(magic.data(), 4);
            std::string magicStr(magic.data(), 4);
            if (magicStr == "WII5") {
                offset = NASOS_WII5_OFFSET;
            } else if (magicStr == "WII9") {
                offset = NASOS_WII9_OFFSET;
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

        size_t offset = GAME_NAME_OFFSET;
        auto fileType = detectFileType(path);

        if (fileType && *fileType == FileType::WBFS) {
            offset = WBFS_GAME_DATA_OFFSET + GAME_NAME_OFFSET;
        } else if (fileType && *fileType == FileType::NASOS) {
            std::array<char, 4> magic{};
            file.read(magic.data(), 4);
            std::string magicStr(magic.data(), 4);
            if (magicStr == "WII5") {
                offset = NASOS_WII5_OFFSET + GAME_NAME_OFFSET;
            } else if (magicStr == "WII9") {
                offset = NASOS_WII9_OFFSET + GAME_NAME_OFFSET;
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

        size_t offset = GAME_TYPE_OFFSET;
        auto fileType = detectFileType(path);

        if (fileType && *fileType == FileType::WBFS) {
            offset = WBFS_GAME_DATA_OFFSET + GAME_TYPE_OFFSET;
        } else if (fileType && *fileType == FileType::NASOS) {
            std::array<char, 4> magic{};
            file.read(magic.data(), 4);
            std::string magicStr(magic.data(), 4);
            if (magicStr == "WII5") {
                offset = NASOS_WII5_OFFSET + GAME_TYPE_OFFSET;
            } else if (magicStr == "WII9") {
                offset = NASOS_WII9_OFFSET + GAME_TYPE_OFFSET;
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
