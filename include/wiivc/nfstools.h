// SPDX-License-Identifier: MIT
#pragma once

#include "types.h"
#include <filesystem>
#include <string>

namespace wiivc::nfstools {

    // Wrapper for nfs2iso2nfs operations
    class NfsTool {
      public:
        // Set the path to nfs2iso2nfs executable (auto-detected if not set)
        void setExecutablePath(const std::filesystem::path &path);

        // Get current executable path
        [[nodiscard]] std::filesystem::path getExecutablePath() const;

        // Check if nfs2iso2nfs is available
        [[nodiscard]] Result<bool> isAvailable();

        // Convert ISO to NFS format
        [[nodiscard]] Result<void> isoToNfs(const std::filesystem::path &isoPath,
                                             const std::filesystem::path &outputDir,
                                             const std::filesystem::path &keyFile,
                                             bool verbose = false);

        // Convert NFS to ISO format
        [[nodiscard]] Result<void> nfsToIso(const std::filesystem::path &nfsDir,
                                             const std::filesystem::path &outputIso,
                                             const std::filesystem::path &keyFile,
                                             bool verbose = false);

        // Encrypt ISO for Wii U
        [[nodiscard]] Result<void> encryptISO(const std::filesystem::path &isoPath,
                                               const std::filesystem::path &outputDir,
                                               const std::filesystem::path &keyFile,
                                               bool passthrough = false,
                                               bool verbose = false);

      private:
        std::filesystem::path executablePath;
        bool autoDetected{false};

        [[nodiscard]] Result<void> autoDetectNfs();
    };

} // namespace wiivc::nfstools
