// SPDX-License-Identifier: MIT
#include "internal/crypto.h"
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <openssl/evp.h>
#include <sstream>

namespace wiivc::crypto {

    namespace {
        std::string formatMD5Hash(const unsigned char *hash, size_t length) {
            std::ostringstream oss;
            for (size_t i = 0; i < length; ++i) {
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
        unsigned char hash[EVP_MAX_MD_SIZE];
        unsigned int hashLength = 0;

        EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
        if (!mdctx) {
            return std::unexpected(ErrorCode::EncryptionError);
        }

        if (EVP_DigestInit_ex(mdctx, EVP_md5(), nullptr) != 1 ||
            EVP_DigestUpdate(mdctx, data.data(), data.size()) != 1 ||
            EVP_DigestFinal_ex(mdctx, hash, &hashLength) != 1) {
            EVP_MD_CTX_free(mdctx);
            return std::unexpected(ErrorCode::EncryptionError);
        }

        EVP_MD_CTX_free(mdctx);
        return formatMD5Hash(hash, hashLength);
    }

    Result<std::string> computeMD5(std::string_view data) {
        unsigned char hash[EVP_MAX_MD_SIZE];
        unsigned int hashLength = 0;

        EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
        if (!mdctx) {
            return std::unexpected(ErrorCode::EncryptionError);
        }

        if (EVP_DigestInit_ex(mdctx, EVP_md5(), nullptr) != 1 ||
            EVP_DigestUpdate(mdctx, data.data(), data.size()) != 1 ||
            EVP_DigestFinal_ex(mdctx, hash, &hashLength) != 1) {
            EVP_MD_CTX_free(mdctx);
            return std::unexpected(ErrorCode::EncryptionError);
        }

        EVP_MD_CTX_free(mdctx);
        return formatMD5Hash(hash, hashLength);
    }

    Result<std::string> computeMD5(const std::filesystem::path &file) {
        std::ifstream stream(file, std::ios::binary);
        if (!stream) {
            return std::unexpected(ErrorCode::FileNotFound);
        }

        EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
        if (!mdctx) {
            return std::unexpected(ErrorCode::EncryptionError);
        }

        if (EVP_DigestInit_ex(mdctx, EVP_md5(), nullptr) != 1) {
            EVP_MD_CTX_free(mdctx);
            return std::unexpected(ErrorCode::EncryptionError);
        }

        constexpr size_t bufferSize = 8192;
        std::array<char, bufferSize> buffer;

        while (stream.read(buffer.data(), buffer.size()) || stream.gcount() > 0) {
            if (EVP_DigestUpdate(mdctx, buffer.data(), stream.gcount()) != 1) {
                EVP_MD_CTX_free(mdctx);
                return std::unexpected(ErrorCode::EncryptionError);
            }
        }

        unsigned char hash[EVP_MAX_MD_SIZE];
        unsigned int hashLength = 0;
        if (EVP_DigestFinal_ex(mdctx, hash, &hashLength) != 1) {
            EVP_MD_CTX_free(mdctx);
            return std::unexpected(ErrorCode::EncryptionError);
        }

        EVP_MD_CTX_free(mdctx);
        return formatMD5Hash(hash, hashLength);
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
