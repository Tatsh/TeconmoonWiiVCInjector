// SPDX-License-Identifier: MIT
#include "wiivc/crypto.h"
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <openssl/md5.h>
#include <sstream>

namespace wiivc::crypto {

    namespace {
        std::string formatMD5Hash(const unsigned char *hash) {
            std::ostringstream oss;
            for (int i = 0; i < MD5_DIGEST_LENGTH; ++i) {
                if (i > 0) {
                    oss << "-";
                }
                oss << std::hex << std::uppercase << std::setw(2) << std::setfill('0')
                    << static_cast<int>(hash[i]);
            }
            return oss.str();
        }

        std::string normalizeHash(std::string_view hash) {
            std::string result;
            for (char c : hash) {
                if (c != '-' && c != ' ') {
                    result.push_back(std::toupper(c));
                }
            }
            return result;
        }
    } // namespace

    Result<std::string> computeMD5(const std::vector<uint8_t> &data) {
        unsigned char hash[MD5_DIGEST_LENGTH];
        MD5(data.data(), data.size(), hash);
        return formatMD5Hash(hash);
    }

    Result<std::string> computeMD5(std::string_view data) {
        unsigned char hash[MD5_DIGEST_LENGTH];
        MD5(reinterpret_cast<const unsigned char *>(data.data()), data.size(), hash);
        return formatMD5Hash(hash);
    }

    Result<std::string> computeMD5(const std::filesystem::path &file) {
        std::ifstream stream(file, std::ios::binary);
        if (!stream) {
            return std::unexpected(ErrorCode::FileNotFound);
        }

        MD5_CTX md5Context;
        MD5_Init(&md5Context);

        constexpr size_t bufferSize = 8192;
        std::array<char, bufferSize> buffer;

        while (stream.read(buffer.data(), buffer.size()) || stream.gcount() > 0) {
            MD5_Update(&md5Context, buffer.data(), stream.gcount());
        }

        unsigned char hash[MD5_DIGEST_LENGTH];
        MD5_Final(hash, &md5Context);

        return formatMD5Hash(hash);
    }

    Result<bool> verifyMD5(const std::vector<uint8_t> &data, std::string_view expectedHash) {
        auto hashResult = computeMD5(data);
        if (!hashResult) {
            return std::unexpected(hashResult.error());
        }

        auto computed = normalizeHash(*hashResult);
        auto expected = normalizeHash(expectedHash);

        return computed == expected;
    }

    Result<bool> verifyMD5(const std::filesystem::path &file, std::string_view expectedHash) {
        auto hashResult = computeMD5(file);
        if (!hashResult) {
            return std::unexpected(hashResult.error());
        }

        auto computed = normalizeHash(*hashResult);
        auto expected = normalizeHash(expectedHash);

        return computed == expected;
    }

    Result<bool> verifyWiiUCommonKey(std::string_view key) {
        auto hashResult = computeMD5(key);
        if (!hashResult) {
            return std::unexpected(hashResult.error());
        }
        return normalizeHash(*hashResult) == normalizeHash(WIIU_COMMON_KEY_HASH);
    }

    Result<bool> verifyTitleKey(std::string_view key) {
        auto hashResult = computeMD5(key);
        if (!hashResult) {
            return std::unexpected(hashResult.error());
        }
        return normalizeHash(*hashResult) == normalizeHash(TITLE_KEY_HASH);
    }

    Result<bool> verifyAncastKey(std::string_view key) {
        auto hashResult = computeMD5(key);
        if (!hashResult) {
            return std::unexpected(hashResult.error());
        }
        return normalizeHash(*hashResult) == normalizeHash(ANCAST_KEY_HASH);
    }

} // namespace wiivc::crypto
