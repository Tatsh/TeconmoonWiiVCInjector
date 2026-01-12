// Copyright (c) 2025 - Wii VC Injector C++ Port
// SPDX-License-Identifier: MIT
#include "wiivc/wuppackage.h"
#include "wiivc/crypto.h"
#include <bit>
#include <cstring>
#include <fstream>
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/sha.h>

namespace wiivc::wuppackage {

namespace {
// Buffer size for SHA256 hashing
constexpr size_t SHA256_BUFFER_SIZE = 65536;

// Title key size in bytes
constexpr size_t TITLE_KEY_SIZE = 16;

// Helper to write big-endian values
template <typename T> void writeBE(std::vector<uint8_t> &buf, T value) {
    if constexpr (std::endian::native == std::endian::little) {
        value = std::byteswap(value);
    }
    const auto *bytes = reinterpret_cast<const uint8_t *>(&value);
    buf.insert(buf.end(), bytes, bytes + sizeof(T));
}

// Calculate SHA256 hash
std::array<uint8_t, 32> sha256(const std::vector<uint8_t> &data) {
    std::array<uint8_t, 32> hash{};
    SHA256(data.data(), data.size(), hash.data());
    return hash;
}

// Calculate SHA256 hash of file
Result<std::array<uint8_t, 32>> sha256File(const std::filesystem::path &path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return std::unexpected(ErrorCode::FileNotFound);
    }

    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (ctx == nullptr) {
        return std::unexpected(ErrorCode::EncryptionError);
    }

    if (EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) != 1) {
        EVP_MD_CTX_free(ctx);
        return std::unexpected(ErrorCode::EncryptionError);
    }

    std::vector<uint8_t> buffer(SHA256_BUFFER_SIZE);

    while (file.read(reinterpret_cast<char *>(buffer.data()), SHA256_BUFFER_SIZE) || file.gcount() > 0) {
        if (EVP_DigestUpdate(ctx, buffer.data(), file.gcount()) != 1) {
            EVP_MD_CTX_free(ctx);
            return std::unexpected(ErrorCode::EncryptionError);
        }
    }

    std::array<uint8_t, 32> hash{};
    unsigned int hashLen = 0;
    if (EVP_DigestFinal_ex(ctx, hash.data(), &hashLen) != 1) {
        EVP_MD_CTX_free(ctx);
        return std::unexpected(ErrorCode::EncryptionError);
    }

    EVP_MD_CTX_free(ctx);
    return hash;
}

} // namespace

Result<std::vector<uint8_t>>
WUPPackager::encryptTitleKey(const std::array<uint8_t, 16> &titleKey,
                              const std::array<uint8_t, 16> &commonKey, uint64_t titleID) {
    // IV is title ID padded with zeros
    std::array<uint8_t, 16> iv{};
    uint64_t titleIDBE = titleID;
    if constexpr (std::endian::native == std::endian::little) {
        titleIDBE = std::byteswap(titleIDBE);
    }
    std::memcpy(iv.data(), &titleIDBE, 8);

    // Encrypt title key with common key using AES-128-CBC
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (ctx == nullptr) {
        return std::unexpected(ErrorCode::EncryptionError);
    }

    if (EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), nullptr, commonKey.data(), iv.data()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return std::unexpected(ErrorCode::EncryptionError);
    }

    std::vector<uint8_t> encrypted(32); // Max size for 16 bytes + padding
    int len = 0;
    if (EVP_EncryptUpdate(ctx, encrypted.data(), &len, titleKey.data(), titleKey.size()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return std::unexpected(ErrorCode::EncryptionError);
    }

    int totalLen = len;
    if (EVP_EncryptFinal_ex(ctx, encrypted.data() + len, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return std::unexpected(ErrorCode::EncryptionError);
    }
    totalLen += len;

    EVP_CIPHER_CTX_free(ctx);
    encrypted.resize(TITLE_KEY_SIZE); // Title key is exactly 16 bytes when encrypted
    return encrypted;
}

Result<std::vector<uint8_t>> WUPPackager::generateTicket(const PackageConfig &config) {
    std::vector<uint8_t> ticket;
    ticket.reserve(0x350);

    // Encrypt title key
    auto encKeyResult = encryptTitleKey(config.encryptionKey, config.encryptKeyWith, config.titleID);
    if (!encKeyResult) {
        return std::unexpected(encKeyResult.error());
    }
    const auto &encryptedKey = *encKeyResult;

    // Signature type
    writeBE(ticket, uint32_t(0x00010004));

    // Random signature (256 bytes)
    ticket.resize(ticket.size() + 0x100);

    // Padding
    ticket.resize(ticket.size() + 0x3C);

    // Issuer: "Root-CA00000003-XS0000000c"
    const char issuer[] = "Root-CA00000003-XS0000000c";
    ticket.insert(ticket.end(), issuer, issuer + sizeof(issuer) - 1);
    ticket.resize(ticket.size() + (0x40 - sizeof(issuer) + 1));

    // More padding
    ticket.resize(ticket.size() + 0x5C);

    // Unknown/version bytes
    ticket.push_back(0x01);
    ticket.push_back(0x00);
    ticket.push_back(0x00);

    // Encrypted title key (16 bytes)
    ticket.insert(ticket.end(), encryptedKey.begin(), encryptedKey.begin() + 16);

    // Padding and fields
    ticket.resize(ticket.size() + 5);

    // Random ticket ID (6 bytes)
    ticket.resize(ticket.size() + 6);

    // Console ID
    writeBE(ticket, uint32_t(0));

    // Title ID
    writeBE(ticket, config.titleID);

    // More fields
    const uint8_t ticketData[] = {0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x00,
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x05};
    ticket.insert(ticket.end(), ticketData, ticketData + sizeof(ticketData));

    // Reserved
    ticket.resize(ticket.size() + 0xB0);

    // Content access permissions
    const uint8_t permissions[] = {
        0x00, 0x01, 0x00, 0x14, 0x00, 0x00, 0x00, 0xAC, 0x00, 0x00, 0x00, 0x14, 0x00, 0x01,
        0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x01,
        0x00, 0x00, 0x00, 0x84, 0x00, 0x00, 0x00, 0x84, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x01};
    ticket.insert(ticket.end(), permissions, permissions + sizeof(permissions));

    // Final padding to 0x350
    ticket.resize(0x350);

    return ticket;
}

Result<std::vector<uint8_t>>
WUPPackager::generateTMD(const PackageConfig &config,
                          const std::vector<ContentRecord> &contents) {
    std::vector<uint8_t> tmd;

    // Signature type
    writeBE(tmd, uint32_t(0x00010004));

    // Signature (256 bytes)
    tmd.resize(tmd.size() + 0x100);

    // Padding
    tmd.resize(tmd.size() + 0x3C);

    // Issuer: "Root-CA00000003-CP0000000b"
    const char issuer[] = "Root-CA00000003-CP0000000b";
    tmd.insert(tmd.end(), issuer, issuer + sizeof(issuer) - 1);
    tmd.resize(tmd.size() + (0x40 - sizeof(issuer) + 1));

    // Version fields
    tmd.push_back(0x01); // version
    tmd.push_back(0x00); // CA CRL version
    tmd.push_back(0x00); // Signer CRL version
    tmd.push_back(0x00); // padding

    // System version
    writeBE(tmd, config.osVersion);

    // Title ID
    writeBE(tmd, config.titleID);

    // Title type
    writeBE(tmd, uint32_t(0x00010000));

    // Group ID
    writeBE(tmd, config.groupID);

    // App type
    writeBE(tmd, config.appType);

    // Reserved fields
    writeBE(tmd, uint32_t(0));
    writeBE(tmd, uint32_t(0));
    tmd.resize(tmd.size() + 50);

    // Access rights
    writeBE(tmd, uint32_t(0));

    // Title version
    writeBE(tmd, config.titleVersion);

    // Content count
    writeBE(tmd, uint16_t(contents.size()));

    // Boot index
    writeBE(tmd, uint16_t(0));

    // Padding
    writeBE(tmd, uint16_t(0));

    // SHA2 hash placeholder (will be updated later)
    tmd.resize(tmd.size() + 0x20);

    // Content info (simplified - one content info covering all contents)
    writeBE(tmd, uint16_t(0));                     // index offset
    writeBE(tmd, uint16_t(contents.size()));       // command count
    tmd.resize(tmd.size() + 0x20);                 // SHA2 hash placeholder

    // Pad to 64 content infos
    tmd.resize(tmd.size() + (64 - 1) * (2 + 2 + 0x20));

    // Content records
    for (const auto &content : contents) {
        writeBE(tmd, content.contentID);
        writeBE(tmd, content.index);
        writeBE(tmd, content.type);
        writeBE(tmd, content.size);
        tmd.insert(tmd.end(), content.sha2Hash.begin(), content.sha2Hash.end());
    }

    return tmd;
}

Result<FSTEntry> WUPPackager::buildFST(const std::filesystem::path &dir) {
    FSTEntry root;
    root.isDirectory = true;
    root.name = "";
    root.offset = 0;
    root.size = 0;
    root.contentID = 0;

    if (!std::filesystem::exists(dir)) {
        return std::unexpected(ErrorCode::FileNotFound);
    }

    // Simplified FST building - just enumerate files
    for (const auto &entry : std::filesystem::recursive_directory_iterator(dir)) {
        if (entry.is_regular_file()) {
            FSTEntry fileEntry;
            fileEntry.isDirectory = false;
            fileEntry.name = entry.path().filename().string();
            fileEntry.offset = 0;
            fileEntry.size = static_cast<uint32_t>(std::filesystem::file_size(entry.path()));
            fileEntry.contentID = 0;
            root.children.push_back(fileEntry);
        }
    }

    return root;
}

Result<std::vector<ContentRecord>> WUPPackager::generateContentRecords(const FSTEntry &root) {
    std::vector<ContentRecord> records;

    // Simplified: Create one content record for each major directory (code, content, meta)
    ContentRecord codeRecord{};
    codeRecord.contentID = 0;
    codeRecord.index = 0;
    codeRecord.type = 0x0001; // Encrypted
    codeRecord.size = 0;      // Will be set when encrypting
    records.push_back(codeRecord);

    return records;
}

Result<void> WUPPackager::encryptContent(const std::filesystem::path &inputPath,
                                          const std::filesystem::path &outputPath,
                                          uint16_t contentID,
                                          const std::array<uint8_t, 16> &key) {
    // Read input file
    std::ifstream inFile(inputPath, std::ios::binary);
    if (!inFile) {
        return std::unexpected(ErrorCode::FileNotFound);
    }

    std::vector<uint8_t> data((std::istreambuf_iterator<char>(inFile)),
                               std::istreambuf_iterator<char>());

    // IV is content ID in first 2 bytes, rest zeros
    std::array<uint8_t, 16> iv{};
    uint16_t contentIDBE = contentID;
    if constexpr (std::endian::native == std::endian::little) {
        contentIDBE = std::byteswap(contentIDBE);
    }
    std::memcpy(iv.data(), &contentIDBE, 2);

    // Encrypt with AES-128-CBC
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (ctx == nullptr) {
        return std::unexpected(ErrorCode::EncryptionError);
    }

    if (EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), nullptr, key.data(), iv.data()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return std::unexpected(ErrorCode::EncryptionError);
    }

    // Disable padding - pad manually to 16-byte boundary
    EVP_CIPHER_CTX_set_padding(ctx, 0);

    size_t paddedSize = ((data.size() + 15) / 16) * 16;
    data.resize(paddedSize);

    std::vector<uint8_t> encrypted(paddedSize + 16);
    int len = 0;
    if (EVP_EncryptUpdate(ctx, encrypted.data(), &len, data.data(), data.size()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return std::unexpected(ErrorCode::EncryptionError);
    }

    int totalLen = len;
    if (EVP_EncryptFinal_ex(ctx, encrypted.data() + len, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return std::unexpected(ErrorCode::EncryptionError);
    }
    totalLen += len;

    EVP_CIPHER_CTX_free(ctx);
    encrypted.resize(totalLen);

    // Write encrypted file
    std::ofstream outFile(outputPath, std::ios::binary);
    if (!outFile) {
        return std::unexpected(ErrorCode::IOError);
    }

    outFile.write(reinterpret_cast<const char *>(encrypted.data()), encrypted.size());
    if (!outFile) {
        return std::unexpected(ErrorCode::IOError);
    }

    return {};
}

Result<void> WUPPackager::createPackage(const PackageConfig &config) {
    // Verify input directories exist
    auto codeDir = config.inputDir / "code";
    auto contentDir = config.inputDir / "content";
    auto metaDir = config.inputDir / "meta";

    if (!std::filesystem::exists(codeDir) || !std::filesystem::exists(contentDir) ||
        !std::filesystem::exists(metaDir)) {
        return std::unexpected(ErrorCode::FileNotFound);
    }

    // Create output directory
    std::filesystem::create_directories(config.outputDir);

    // Build FST
    auto fstResult = buildFST(config.inputDir);
    if (!fstResult) {
        return std::unexpected(fstResult.error());
    }

    // Generate content records
    auto contentRecords = generateContentRecords(*fstResult);
    if (!contentRecords) {
        return std::unexpected(contentRecords.error());
    }

    // Generate TMD
    auto tmdResult = generateTMD(config, *contentRecords);
    if (!tmdResult) {
        return std::unexpected(tmdResult.error());
    }

    // Generate Ticket
    auto ticketResult = generateTicket(config);
    if (!ticketResult) {
        return std::unexpected(ticketResult.error());
    }

    // Write TMD
    auto tmdPath = config.outputDir / "title.tmd";
    std::ofstream tmdFile(tmdPath, std::ios::binary);
    if (!tmdFile) {
        return std::unexpected(ErrorCode::IOError);
    }
    tmdFile.write(reinterpret_cast<const char *>(tmdResult->data()), tmdResult->size());

    // Write Ticket
    auto ticketPath = config.outputDir / "title.tik";
    std::ofstream ticketFile(ticketPath, std::ios::binary);
    if (!ticketFile) {
        return std::unexpected(ErrorCode::IOError);
    }
    ticketFile.write(reinterpret_cast<const char *>(ticketResult->data()), ticketResult->size());

    // Note: Full implementation would encrypt and write .app files
    // This is a simplified version showing the core structure

    return {};
}

} // namespace wiivc::wuppackage
