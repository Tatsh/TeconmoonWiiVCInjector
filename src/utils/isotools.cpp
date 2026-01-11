// SPDX-License-Identifier: MIT
#include "wiivc/isotools.h"
#include "wiivc/process.h"
#include "wiivc/stringutils.h"
#include <fmt/core.h>
#include <sstream>

namespace wiivc::isotools {

    void WitTool::setExecutablePath(const std::filesystem::path &path) {
        executablePath = path;
        autoDetected = false;
    }

    std::filesystem::path WitTool::getExecutablePath() const {
        return executablePath;
    }

    Result<void> WitTool::autoDetectWit() {
        if (!executablePath.empty() && !autoDetected) {
            return {}; // Already manually set
        }

        // Try to find wit in PATH
        auto findResult = process::findInPath("wit");
        if (findResult) {
            executablePath = *findResult;
            autoDetected = true;
            return {};
        }

        return std::unexpected(ErrorCode::ToolNotFound);
    }

    Result<bool> WitTool::isAvailable() {
        auto detectResult = autoDetectWit();
        if (!detectResult) {
            return false;
        }

        // Verify wit works by running with --version
        auto result = process::executeSimple(executablePath, {"--version"});
        if (!result) {
            return false;
        }

        return *result == 0;
    }

    Result<void> WitTool::extractISO(const std::filesystem::path &isoPath,
                                      const std::filesystem::path &outputDir,
                                      bool verbose) {
        auto detectResult = autoDetectWit();
        if (!detectResult) {
            return detectResult;
        }

        if (!std::filesystem::exists(isoPath)) {
            return std::unexpected(ErrorCode::FileNotFound);
        }

        std::vector<std::string> args = {"extract", isoPath.string(), "--DEST", outputDir.string()};

        if (verbose) {
            args.push_back("-v");
        }

        auto result = process::execute(
            executablePath,
            args,
            {},
            verbose ? [](std::string_view line) { fmt::print("wit: {}", line); } : nullptr);

        if (!result) {
            return std::unexpected(result.error());
        }

        if (result->exitCode != 0) {
            return std::unexpected(ErrorCode::IOError);
        }

        return {};
    }

    Result<void> WitTool::convertFormat(const std::filesystem::path &inputPath,
                                         const std::filesystem::path &outputPath,
                                         std::string_view targetFormat,
                                         bool verbose) {
        auto detectResult = autoDetectWit();
        if (!detectResult) {
            return detectResult;
        }

        if (!std::filesystem::exists(inputPath)) {
            return std::unexpected(ErrorCode::FileNotFound);
        }

        std::vector<std::string> args = {"copy", inputPath.string(), outputPath.string()};

        // Add format conversion
        if (!targetFormat.empty()) {
            args.push_back(fmt::format("--{}", targetFormat));
        }

        if (verbose) {
            args.push_back("-v");
        }

        auto result = process::execute(
            executablePath,
            args,
            {},
            verbose ? [](std::string_view line) { fmt::print("wit: {}", line); } : nullptr);

        if (!result) {
            return std::unexpected(result.error());
        }

        if (result->exitCode != 0) {
            return std::unexpected(ErrorCode::ConversionError);
        }

        return {};
    }

    Result<WitTool::DiscInfo> WitTool::getDiscInfo(const std::filesystem::path &discPath) {
        auto detectResult = autoDetectWit();
        if (!detectResult) {
            return std::unexpected(detectResult.error());
        }

        if (!std::filesystem::exists(discPath)) {
            return std::unexpected(ErrorCode::FileNotFound);
        }

        std::vector<std::string> args = {"list", "-l", discPath.string()};

        auto result = process::execute(executablePath, args);

        if (!result) {
            return std::unexpected(result.error());
        }

        if (result->exitCode != 0) {
            return std::unexpected(ErrorCode::IOError);
        }

        // Parse wit output
        DiscInfo info;
        std::istringstream stream(result->stdoutOutput);
        std::string line;

        while (std::getline(stream, line)) {
            // Parse lines like "ID: RMCE01"
            if (line.find("ID:") != std::string::npos) {
                auto pos = line.find(':');
                if (pos != std::string::npos) {
                    info.gameId = line.substr(pos + 1);
                    // Trim whitespace
                    info.gameId.erase(0, info.gameId.find_first_not_of(" \t"));
                    info.gameId.erase(info.gameId.find_last_not_of(" \t") + 1);
                }
            }
            // Parse other fields as needed
        }

        return info;
    }

    Result<void> WitTool::trimISO(const std::filesystem::path &isoPath,
                                   const std::filesystem::path &outputPath,
                                   bool verbose) {
        auto detectResult = autoDetectWit();
        if (!detectResult) {
            return detectResult;
        }

        if (!std::filesystem::exists(isoPath)) {
            return std::unexpected(ErrorCode::FileNotFound);
        }

        std::vector<std::string> args = {"copy",
                                          isoPath.string(),
                                          outputPath.string(),
                                          "--trim"};

        if (verbose) {
            args.push_back("-v");
        }

        auto result = process::execute(
            executablePath,
            args,
            {},
            verbose ? [](std::string_view line) { fmt::print("wit: {}", line); } : nullptr);

        if (!result) {
            return std::unexpected(result.error());
        }

        if (result->exitCode != 0) {
            return std::unexpected(ErrorCode::ConversionError);
        }

        return {};
    }

} // namespace wiivc::isotools
