// SPDX-License-Identifier: MIT
#pragma once

#include <cstdint>
#include <expected>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

namespace wiivc {

    // Error codes for operations without exceptions
    enum class ErrorCode {
        Success = 0,
        FileNotFound,
        InvalidFile,
        InvalidFormat,
        IOError,
        NetworkError,
        HashMismatch,
        InsufficientSpace,
        ToolNotFound,
        EncryptionError,
        ConversionError
    };

    // Convert error code to string
    [[nodiscard]] constexpr std::string_view errorToString(ErrorCode code) noexcept {
        switch (code) {
            case ErrorCode::Success:
                return "Success";
            case ErrorCode::FileNotFound:
                return "File not found";
            case ErrorCode::InvalidFile:
                return "Invalid file";
            case ErrorCode::InvalidFormat:
                return "Invalid format";
            case ErrorCode::IOError:
                return "I/O error";
            case ErrorCode::NetworkError:
                return "Network error";
            case ErrorCode::HashMismatch:
                return "Hash mismatch";
            case ErrorCode::InsufficientSpace:
                return "Insufficient disk space";
            case ErrorCode::ToolNotFound:
                return "Required tool not found";
            case ErrorCode::EncryptionError:
                return "Encryption error";
            case ErrorCode::ConversionError:
                return "Conversion error";
            default:
                return "Unknown error";
        }
    }

    // Result type for operations
    template<typename T>
    using Result = std::expected<T, ErrorCode>;

    // System type enumeration
    enum class SystemType { WiiRetail, WiiHomebrew, WiiNAND, GameCubeRetail };

    // Image format specification
    struct ImageFormat {
        uint32_t width;
        uint32_t height;
        uint32_t bpp;
        bool compression;
    };

    // File type detection
    enum class FileType { Unknown, ISO, WBFS, NKIT, NASOS, DOL, GCM };

} // namespace wiivc
