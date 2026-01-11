// SPDX-License-Identifier: MIT
#pragma once

#include "types.h"
#include <array>
#include <cstdint>
#include <filesystem>
#include <vector>

namespace wiivc::nfsconvert {

    // Constants from nfs2iso2nfs
    constexpr size_t SECTOR_SIZE = 0x8000;
    constexpr size_t HEADER_SIZE = 0x200;
    constexpr size_t NFS_SIZE = 0xFA00000;

    // NFS converter class - port of nfs2iso2nfs C# code
    class NfsConverter {
      public:
        // Convert ISO to NFS format
        [[nodiscard]] Result<void> isoToNfs(const std::filesystem::path &isoPath,
                                             const std::filesystem::path &nfsDir,
                                             const std::vector<uint8_t> &encryptionKey,
                                             bool keepIntermediateFiles = false,
                                             bool keepLegitSignature = false);

        // Convert NFS to ISO format
        [[nodiscard]] Result<void> nfsToIso(const std::filesystem::path &nfsDir,
                                             const std::filesystem::path &isoPath,
                                             const std::vector<uint8_t> &decryptionKey,
                                             bool keepIntermediateFiles = false);

        // Set firmware image path (for fake signing patch)
        void setFirmwarePath(const std::filesystem::path &path);

      private:
        std::filesystem::path firmwarePath;

        // Core NFS operations
        [[nodiscard]] Result<std::vector<uint8_t>> getHeader(const std::filesystem::path &nfsFile);
        [[nodiscard]] Result<void> combineNfsFiles(const std::filesystem::path &outputFile,
                                                    const std::filesystem::path &nfsDir);
        [[nodiscard]] Result<void> splitNfsFile(const std::filesystem::path &inputFile,
                                                 const std::filesystem::path &nfsDir);
        
        // Encryption/Decryption
        [[nodiscard]] Result<void> encryptDecryptNfs(const std::filesystem::path &inputFile,
                                                      const std::filesystem::path &outputFile,
                                                      const std::vector<uint8_t> &key,
                                                      const std::vector<uint8_t> &iv,
                                                      bool encrypt,
                                                      const std::vector<uint8_t> &header);

        // Packing/Unpacking
        [[nodiscard]] Result<void> unpackNfs(const std::filesystem::path &inputFile,
                                              const std::filesystem::path &outputFile,
                                              const std::vector<uint8_t> &header);
        
        [[nodiscard]] Result<std::vector<uint8_t>> packNfs(const std::filesystem::path &inputFile,
                                                            const std::filesystem::path &outputFile,
                                                            const std::array<int64_t, 2> &sizes);

        // ISO manipulation
        [[nodiscard]] Result<std::array<int64_t, 2>> manipulateIso(
            const std::filesystem::path &inputFile,
            const std::filesystem::path &outputFile,
            bool decrypt);

        // Patching
        [[nodiscard]] Result<void> patchFakeSigning(const std::filesystem::path &fwFile);

        // Helper functions
        [[nodiscard]] static std::vector<uint8_t> buildZero(size_t length);
    };

} // namespace wiivc::nfsconvert
