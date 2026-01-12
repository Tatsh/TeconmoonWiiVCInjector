// SPDX-License-Identifier: MIT
#pragma once

#include <algorithm>
#include <cctype>
#include <locale>
#include <string>
#include <string_view>
#include <vector>
#include <cstdint>

namespace wiivc::utils {

    // Remove diacritics from string
    [[nodiscard]] inline std::string removeDiacritics(std::string_view text) {
        // For full Unicode support, would need ICU library
        // Simple ASCII version for now
        std::string result;
        result.reserve(text.size());
        for (char c : text) {
            if (static_cast<unsigned char>(c) < 128) {
                result.push_back(c);
            }
        }
        return result;
    }

    // Remove special characters
    [[nodiscard]] inline std::string removeSpecialChars(std::string_view v) {
        if (v.empty()) {
            return std::string(v);
        }
        auto s = removeDiacritics(v);
        std::string stripped;
        stripped.reserve(s.size());
        for (char c : s) {
            if (static_cast<unsigned char>(c) < 128) {
                stripped.push_back(c);
            }
        }
        return stripped;
    }

    // Replace character at index
    [[nodiscard]] inline std::string replaceAt(std::string input, size_t index, char newChar) {
        if (index >= input.size()) {
            return input;
        }
        input[index] = newChar;
        return input;
    }

    // Convert hex string to bytes
    [[nodiscard]] inline std::vector<uint8_t> hexToBytes(std::string_view hex) {
        std::vector<uint8_t> bytes;
        bytes.reserve(hex.size() / 2);
        for (size_t i = 0; i + 1 < hex.size(); i += 2) {
            auto byte =
                static_cast<uint8_t>(std::stoi(std::string(hex.substr(i, 2)), nullptr, 16));
            bytes.push_back(byte);
        }
        return bytes;
    }

    // Convert bytes to hex string
    [[nodiscard]] inline std::string bytesToHex(const std::vector<uint8_t> &bytes) {
        static constexpr char hexChars[] = "0123456789ABCDEF";
        std::string result;
        result.reserve(bytes.size() * 2);
        for (uint8_t byte : bytes) {
            result.push_back(hexChars[byte >> 4]);
            result.push_back(hexChars[byte & 0x0F]);
        }
        return result;
    }

    // Sanitize filename (remove invalid characters)
    [[nodiscard]] inline std::string sanitizeFilename(std::string_view str) {
        static constexpr char invalidChars[] = "<>:\"/\\|?*";
        std::string result;
        result.reserve(str.size());
        for (char c : str) {
            if (std::find(std::begin(invalidChars),
                          std::end(invalidChars) - 1,
                          c) == std::end(invalidChars) - 1) {
                result.push_back(c);
            } else {
                result.push_back('_');
            }
        }
        // Remove trailing dots
        while (!result.empty() && result.back() == '.') {
            result.pop_back();
        }
        return result;
    }

} // namespace wiivc::utils
