// SPDX-License-Identifier: MIT
#pragma once

#include "types.h"
#include <array>
#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace wiivc::crypto {

    // MD5 hash computation
    [[nodiscard]] Result<std::string> computeMD5(const std::vector<uint8_t> &data);
    [[nodiscard]] Result<std::string> computeMD5(const std::filesystem::path &file);
    [[nodiscard]] Result<std::string> computeMD5(std::string_view data);

    // Verify MD5 hash
    [[nodiscard]] Result<bool> verifyMD5(const std::vector<uint8_t> &data,
                                          std::string_view expectedHash);
    [[nodiscard]] Result<bool> verifyMD5(const std::filesystem::path &file,
                                          std::string_view expectedHash);

    // Known key hashes for verification (from C# code)
    constexpr std::string_view WIIU_COMMON_KEY_HASH = "35-AC-59-94-97-22-79-33-1D-97-09-4F-A2-FB-97-FC";
    constexpr std::string_view TITLE_KEY_HASH = "F9-4B-D8-8E-BB-7A-A9-38-67-E6-30-61-5F-27-1C-9F";
    constexpr std::string_view ANCAST_KEY_HASH = "31-8D-1F-9D-98-FB-08-E7-7C-7F-E1-77-AA-49-05-43";

    // Verify encryption keys
    [[nodiscard]] Result<bool> verifyWiiUCommonKey(std::string_view key);
    [[nodiscard]] Result<bool> verifyTitleKey(std::string_view key);
    [[nodiscard]] Result<bool> verifyAncastKey(std::string_view key);

} // namespace wiivc::crypto
