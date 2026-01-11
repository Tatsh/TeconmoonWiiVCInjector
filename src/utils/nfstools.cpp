// SPDX-License-Identifier: MIT
#include "wiivc/nfstools.h"
#include "wiivc/nfsconvert.h"
#include <spdlog/spdlog.h>

namespace wiivc::nfstools {

    void NfsTool::setExecutablePath(const std::filesystem::path &path) {
        executablePath = path;
        autoDetected = false;
    }

    std::filesystem::path NfsTool::getExecutablePath() const {
        return executablePath;
    }

    Result<void> NfsTool::autoDetectNfs() {
        // No longer needed - using library
        return {};
    }

    Result<bool> NfsTool::isAvailable() {
        // Library is always available
        return true;
    }

    Result<void> NfsTool::isoToNfs(const std::filesystem::path &isoPath,
                                    const std::filesystem::path &outputDir,
                                    const std::filesystem::path &keyFile,
                                    bool verbose) {
        if (!std::filesystem::exists(isoPath)) {
            return std::unexpected(ErrorCode::FileNotFound);
        }

        if (!std::filesystem::exists(keyFile)) {
            return std::unexpected(ErrorCode::FileNotFound);
        }

        // Read key file
        std::ifstream keyStream(keyFile, std::ios::binary);
        if (!keyStream) {
            return std::unexpected(ErrorCode::FileNotFound);
        }

        std::vector<uint8_t> key;
        keyStream.seekg(0, std::ios::end);
        key.resize(keyStream.tellg());
        keyStream.seekg(0, std::ios::beg);
        keyStream.read(reinterpret_cast<char *>(key.data()), key.size());

        // Use NFS converter library
        nfsconvert::NfsConverter converter;
        auto result = converter.isoToNfs(isoPath, outputDir, key, false, false);

        if (verbose && result) {
            spdlog::info("NFS conversion completed: {} -> {}",
                       isoPath.string(),
                       outputDir.string());
        }

        return result;
    }

    Result<void> NfsTool::nfsToIso(const std::filesystem::path &nfsDir,
                                    const std::filesystem::path &outputIso,
                                    const std::filesystem::path &keyFile,
                                    bool verbose) {
        if (!std::filesystem::exists(nfsDir)) {
            return std::unexpected(ErrorCode::FileNotFound);
        }

        if (!std::filesystem::exists(keyFile)) {
            return std::unexpected(ErrorCode::FileNotFound);
        }

        // Read key file
        std::ifstream keyStream(keyFile, std::ios::binary);
        if (!keyStream) {
            return std::unexpected(ErrorCode::FileNotFound);
        }

        std::vector<uint8_t> key;
        keyStream.seekg(0, std::ios::end);
        key.resize(keyStream.tellg());
        keyStream.seekg(0, std::ios::beg);
        keyStream.read(reinterpret_cast<char *>(key.data()), key.size());

        // Use NFS converter library
        nfsconvert::NfsConverter converter;
        auto result = converter.nfsToIso(nfsDir, outputIso, key, false);

        if (verbose && result) {
            spdlog::info("ISO extraction completed: {} -> {}",
                       nfsDir.string(),
                       outputIso.string());
        }

        return result;
    }

    Result<void> NfsTool::encryptISO(const std::filesystem::path &isoPath,
                                      const std::filesystem::path &outputDir,
                                      const std::filesystem::path &keyFile,
                                      bool passthrough,
                                      bool verbose) {
        // Use isoToNfs for encryption
        return isoToNfs(isoPath, outputDir, keyFile, verbose);
    }

} // namespace wiivc::nfstools
