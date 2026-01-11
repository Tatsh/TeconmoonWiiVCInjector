// SPDX-License-Identifier: MIT
#include "wiivc/nfstools.h"
#include "wiivc/process.h"
#include <fmt/core.h>

namespace wiivc::nfstools {

    void NfsTool::setExecutablePath(const std::filesystem::path &path) {
        executablePath = path;
        autoDetected = false;
    }

    std::filesystem::path NfsTool::getExecutablePath() const {
        return executablePath;
    }

    Result<void> NfsTool::autoDetectNfs() {
        if (!executablePath.empty() && !autoDetected) {
            return {}; // Already manually set
        }

        // Try to find nfs2iso2nfs in PATH
        auto findResult = process::findInPath("nfs2iso2nfs");
        if (findResult) {
            executablePath = *findResult;
            autoDetected = true;
            return {};
        }

        return std::unexpected(ErrorCode::ToolNotFound);
    }

    Result<bool> NfsTool::isAvailable() {
        auto detectResult = autoDetectNfs();
        if (!detectResult) {
            return false;
        }

        // Verify nfs2iso2nfs works
        auto result = process::executeSimple(executablePath, {"--help"});
        if (!result) {
            return false;
        }

        // Tool returns 0 or 1 for --help, both are valid
        return true;
    }

    Result<void> NfsTool::isoToNfs(const std::filesystem::path &isoPath,
                                    const std::filesystem::path &outputDir,
                                    const std::filesystem::path &keyFile,
                                    bool verbose) {
        auto detectResult = autoDetectNfs();
        if (!detectResult) {
            return detectResult;
        }

        if (!std::filesystem::exists(isoPath)) {
            return std::unexpected(ErrorCode::FileNotFound);
        }

        if (!std::filesystem::exists(keyFile)) {
            return std::unexpected(ErrorCode::FileNotFound);
        }

        // Create output directory
        if (!std::filesystem::exists(outputDir)) {
            try {
                std::filesystem::create_directories(outputDir);
            } catch (...) {
                return std::unexpected(ErrorCode::IOError);
            }
        }

        std::vector<std::string> args = {"-iso",
                                          isoPath.string(),
                                          "-nfs",
                                          outputDir.string(),
                                          "-enc",
                                          "-key",
                                          keyFile.string()};

        auto result = process::execute(
            executablePath,
            args,
            {},
            verbose ? [](std::string_view line) { fmt::print("nfs2iso2nfs: {}", line); } : nullptr);

        if (!result) {
            return std::unexpected(result.error());
        }

        if (result->exitCode != 0) {
            return std::unexpected(ErrorCode::ConversionError);
        }

        return {};
    }

    Result<void> NfsTool::nfsToIso(const std::filesystem::path &nfsDir,
                                    const std::filesystem::path &outputIso,
                                    const std::filesystem::path &keyFile,
                                    bool verbose) {
        auto detectResult = autoDetectNfs();
        if (!detectResult) {
            return detectResult;
        }

        if (!std::filesystem::exists(nfsDir)) {
            return std::unexpected(ErrorCode::FileNotFound);
        }

        if (!std::filesystem::exists(keyFile)) {
            return std::unexpected(ErrorCode::FileNotFound);
        }

        std::vector<std::string> args = {"-nfs",
                                          nfsDir.string(),
                                          "-iso",
                                          outputIso.string(),
                                          "-dec",
                                          "-key",
                                          keyFile.string()};

        auto result = process::execute(
            executablePath,
            args,
            {},
            verbose ? [](std::string_view line) { fmt::print("nfs2iso2nfs: {}", line); } : nullptr);

        if (!result) {
            return std::unexpected(result.error());
        }

        if (result->exitCode != 0) {
            return std::unexpected(ErrorCode::ConversionError);
        }

        return {};
    }

    Result<void> NfsTool::encryptISO(const std::filesystem::path &isoPath,
                                      const std::filesystem::path &outputDir,
                                      const std::filesystem::path &keyFile,
                                      bool passthrough,
                                      bool verbose) {
        auto detectResult = autoDetectNfs();
        if (!detectResult) {
            return detectResult;
        }

        if (!std::filesystem::exists(isoPath)) {
            return std::unexpected(ErrorCode::FileNotFound);
        }

        if (!std::filesystem::exists(keyFile)) {
            return std::unexpected(ErrorCode::FileNotFound);
        }

        // Create output directory
        if (!std::filesystem::exists(outputDir)) {
            try {
                std::filesystem::create_directories(outputDir);
            } catch (...) {
                return std::unexpected(ErrorCode::IOError);
            }
        }

        std::vector<std::string> args = {"-iso", isoPath.string(), "-nfs", outputDir.string()};

        if (passthrough) {
            args.push_back("-passthrough");
        } else {
            args.push_back("-enc");
        }

        args.push_back("-key");
        args.push_back(keyFile.string());

        auto result = process::execute(
            executablePath,
            args,
            {},
            verbose ? [](std::string_view line) { fmt::print("nfs2iso2nfs: {}", line); } : nullptr);

        if (!result) {
            return std::unexpected(result.error());
        }

        if (result->exitCode != 0) {
            return std::unexpected(ErrorCode::EncryptionError);
        }

        return {};
    }

} // namespace wiivc::nfstools
