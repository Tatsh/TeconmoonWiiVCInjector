// Copyright (c) 2025 - Wii VC Injector C++ Port
// SPDX-License-Identifier: MIT
#pragma once

#include "types.h"
#include <array>
#include <cstdint>
#include <filesystem>
#include <vector>

namespace wiivc::wuppackage {

// WUP package structures for Wii U installable content

// TMD (Title Metadata) structures
struct TMDHeader {
    uint32_t signatureType;           // 0x00010004
    std::array<uint8_t, 0x100> signature;
    std::array<uint8_t, 0x3C> padding0;
    std::array<uint8_t, 0x40> issuer; // Root-CA00000003-CP0000000b
    uint8_t version;
    uint8_t caCRLVersion;
    uint8_t signerCRLVersion;
    uint8_t padding1;
    uint64_t systemVersion;
    uint64_t titleID;
    uint32_t titleType;
    uint16_t groupID;
    uint32_t appType;
    uint32_t reserved1;
    uint32_t reserved2;
    std::array<uint8_t, 50> reserved;
    uint32_t accessRights;
    uint16_t titleVersion;
    uint16_t contentCount;
    uint16_t bootIndex;
    std::array<uint8_t, 2> padding3;
    std::array<uint8_t, 0x20> sha2Hash;
};

// Content info structure
struct ContentInfo {
    uint16_t indexOffset;
    uint16_t commandCount;
    std::array<uint8_t, 0x20> sha2Hash;
};

// Content record structure
struct ContentRecord {
    uint32_t contentID;
    uint16_t index;
    uint16_t type;
    uint64_t size;
    std::array<uint8_t, 0x20> sha2Hash;
};

// Ticket structure
struct Ticket {
    uint32_t signatureType; // 0x00010004
    std::array<uint8_t, 0x100> signature;
    std::array<uint8_t, 0x3C> padding0;
    std::array<uint8_t, 0x40> issuer; // Root-CA00000003-XS0000000c
    std::array<uint8_t, 0x20> ecdhData;
    std::array<uint8_t, 0x3C> reserved1;
    std::array<uint8_t, 0x10> encryptedTitleKey;
    uint8_t unknown1;
    uint64_t ticketID;
    uint32_t consoleID;
    uint64_t titleID;
    uint16_t systemAccess;
    uint16_t titleVersion;
    uint32_t permitTitleMask;
    uint32_t permitTitleID;
    uint8_t titleExportAllowed;
    uint8_t commonKeyIndex;
    std::array<uint8_t, 0x30> reserved2;
    std::array<uint8_t, 0x40> contentAccessPermissions;
    std::array<uint8_t, 2> padding1;
    std::array<uint8_t, 0x40> limits;
};

// FST (File System Table) entry
struct FSTEntry {
    bool isDirectory;
    std::string name;
    uint32_t offset;
    uint32_t size;
    uint32_t contentID;
    std::vector<FSTEntry> children;
};

// Package configuration
struct PackageConfig {
    std::filesystem::path inputDir;     // code, content, meta folders
    std::filesystem::path outputDir;    // Where to save .app files
    uint64_t titleID;                   // Title ID (e.g., 0x0005000010101000)
    uint16_t groupID;                   // Group ID
    uint32_t appType;                   // App type (0x80000000 for normal app)
    uint64_t osVersion;                 // OS version
    uint16_t titleVersion;              // Title version
    std::array<uint8_t, 16> encryptionKey;    // Content encryption key
    std::array<uint8_t, 16> encryptKeyWith;   // Key to encrypt encryption key (common key)
};

// Main packaging class
class WUPPackager {
  public:
    WUPPackager() = default;

    // Create a WUP package from input directory
    [[nodiscard]] Result<void> createPackage(const PackageConfig &config);

    // Generate TMD file
    [[nodiscard]] Result<std::vector<uint8_t>>
    generateTMD(const PackageConfig &config, const std::vector<ContentRecord> &contents);

    // Generate Ticket file
    [[nodiscard]] Result<std::vector<uint8_t>> generateTicket(const PackageConfig &config);

    // Build FST from directory
    [[nodiscard]] Result<FSTEntry> buildFST(const std::filesystem::path &dir);

    // Encrypt and save content file
    [[nodiscard]] Result<void> encryptContent(const std::filesystem::path &inputPath,
                                               const std::filesystem::path &outputPath,
                                               uint16_t contentID,
                                               const std::array<uint8_t, 16> &key);

    // Generate content records
    [[nodiscard]] Result<std::vector<ContentRecord>>
    generateContentRecords(const FSTEntry &root);

  private:
    [[nodiscard]] Result<std::vector<uint8_t>>
    encryptTitleKey(const std::array<uint8_t, 16> &titleKey,
                    const std::array<uint8_t, 16> &commonKey, uint64_t titleID);
};

} // namespace wiivc::wuppackage
