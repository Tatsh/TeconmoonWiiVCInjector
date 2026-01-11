// SPDX-License-Identifier: MIT
#include "wiivc/process.h"
#include <array>
#include <cstdio>
#include <memory>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace wiivc::process {

    Result<ProcessResult> execute(const std::filesystem::path &executable,
                                   const std::vector<std::string> &args,
                                   const std::filesystem::path &workingDir,
                                   OutputCallback stdoutCallback,
                                   OutputCallback stderrCallback) {
        if (!std::filesystem::exists(executable)) {
            return std::unexpected(ErrorCode::ToolNotFound);
        }

        // Build command line
        std::ostringstream cmdStream;
        cmdStream << executable.string();
        for (const auto &arg : args) {
            cmdStream << " \"" << arg << "\"";
        }
        std::string command = cmdStream.str();

        ProcessResult result;

#ifdef _WIN32
        // Windows implementation using popen
        FILE *pipe = _popen(command.c_str(), "r");
        if (!pipe) {
            return std::unexpected(ErrorCode::IOError);
        }

        std::array<char, 128> buffer;
        while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
            std::string line(buffer.data());
            result.stdoutOutput += line;
            if (stdoutCallback) {
                stdoutCallback(line);
            }
        }

        result.exitCode = _pclose(pipe);
#else
        // Unix implementation using popen
        FILE *pipe = popen(command.c_str(), "r");
        if (!pipe) {
            return std::unexpected(ErrorCode::IOError);
        }

        std::array<char, 128> buffer;
        while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
            std::string line(buffer.data());
            result.stdoutOutput += line;
            if (stdoutCallback) {
                stdoutCallback(line);
            }
        }

        int status = pclose(pipe);
        if (WIFEXITED(status)) {
            result.exitCode = WEXITSTATUS(status);
        } else {
            result.exitCode = -1;
        }
#endif

        return result;
    }

    Result<int> executeSimple(const std::filesystem::path &executable,
                               const std::vector<std::string> &args,
                               const std::filesystem::path &workingDir) {
        auto result = execute(executable, args, workingDir);
        if (!result) {
            return std::unexpected(result.error());
        }
        return result->exitCode;
    }

    Result<bool> isExecutable(const std::filesystem::path &path) {
        if (!std::filesystem::exists(path)) {
            return false;
        }

#ifdef _WIN32
        // On Windows, check if it's a file with .exe, .bat, .cmd extension
        auto ext = path.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        return ext == ".exe" || ext == ".bat" || ext == ".cmd";
#else
        // On Unix, check if file has execute permission
        return (access(path.c_str(), X_OK) == 0);
#endif
    }

    Result<std::filesystem::path> findInPath(std::string_view executableName) {
#ifdef _WIN32
        const char *pathEnv = std::getenv("PATH");
        if (!pathEnv) {
            return std::unexpected(ErrorCode::ToolNotFound);
        }

        std::string pathStr(pathEnv);
        std::istringstream pathStream(pathStr);
        std::string dir;

        while (std::getline(pathStream, dir, ';')) {
            auto exePath = std::filesystem::path(dir) / executableName;
            
            // Try with .exe extension if not present
            if (exePath.extension().empty()) {
                exePath.replace_extension(".exe");
            }

            if (std::filesystem::exists(exePath)) {
                auto execResult = isExecutable(exePath);
                if (execResult && *execResult) {
                    return exePath;
                }
            }
        }
#else
        const char *pathEnv = std::getenv("PATH");
        if (!pathEnv) {
            return std::unexpected(ErrorCode::ToolNotFound);
        }

        std::string pathStr(pathEnv);
        std::istringstream pathStream(pathStr);
        std::string dir;

        while (std::getline(pathStream, dir, ':')) {
            auto exePath = std::filesystem::path(dir) / executableName;
            if (std::filesystem::exists(exePath)) {
                auto execResult = isExecutable(exePath);
                if (execResult && *execResult) {
                    return exePath;
                }
            }
        }
#endif

        return std::unexpected(ErrorCode::ToolNotFound);
    }

} // namespace wiivc::process
