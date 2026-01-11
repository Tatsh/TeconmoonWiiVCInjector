// SPDX-License-Identifier: MIT
#pragma once

#include "types.h"
#include <filesystem>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace wiivc::process {

    // Process execution result
    struct ProcessResult {
        int exitCode{0};
        std::string stdoutOutput;
        std::string stderrOutput;
    };

    // Output callback for real-time output processing
    using OutputCallback = std::function<void(std::string_view)>;

    // Execute a process and wait for completion
    [[nodiscard]] Result<ProcessResult> execute(const std::filesystem::path &executable,
                                                 const std::vector<std::string> &args,
                                                 const std::filesystem::path &workingDir = {},
                                                 OutputCallback stdoutCallback = nullptr,
                                                 OutputCallback stderrCallback = nullptr);

    // Execute and return only exit code (for simple cases)
    [[nodiscard]] Result<int> executeSimple(const std::filesystem::path &executable,
                                             const std::vector<std::string> &args,
                                             const std::filesystem::path &workingDir = {});

    // Check if executable exists and is executable
    [[nodiscard]] Result<bool> isExecutable(const std::filesystem::path &path);

    // Find executable in PATH
    [[nodiscard]] Result<std::filesystem::path> findInPath(std::string_view executableName);

} // namespace wiivc::process
