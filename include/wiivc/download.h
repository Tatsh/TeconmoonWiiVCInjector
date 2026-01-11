// SPDX-License-Identifier: MIT
#pragma once

#include "types.h"
#include <cstdint>
#include <filesystem>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace wiivc::download {

    // Progress callback: (bytesDownloaded, totalBytes) -> bool (return false to cancel)
    using ProgressCallback = std::function<bool(size_t, size_t)>;

    // Download file from URL to local path
    [[nodiscard]] Result<void> downloadFile(std::string_view url,
                                             const std::filesystem::path &outputPath,
                                             ProgressCallback progress = nullptr);

    // Download to memory
    [[nodiscard]] Result<std::vector<uint8_t>> downloadToMemory(std::string_view url,
                                                                 ProgressCallback progress = nullptr);

    // Check if URL is accessible (HEAD request)
    [[nodiscard]] Result<bool> urlExists(std::string_view url);

    // Get file size from URL without downloading
    [[nodiscard]] Result<size_t> getRemoteFileSize(std::string_view url);

    // Download base files from Nintendo CDN
    struct BaseFileDownloader {
        std::string commonKey;
        std::string titleKey;
        std::filesystem::path cacheDir;

        // Download and cache base files needed for injection
        [[nodiscard]] Result<void> downloadBaseFiles(ProgressCallback progress = nullptr);

        // Check if base files are already cached
        [[nodiscard]] Result<bool> areBaseFilesCached();
    };

} // namespace wiivc::download
