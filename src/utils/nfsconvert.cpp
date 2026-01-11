// SPDX-License-Identifier: MIT
#include "wiivc/nfsconvert.h"
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <cstring>
#include <fstream>

namespace wiivc::nfsconvert {

    void NfsConverter::setFirmwarePath(const std::filesystem::path &path) {
        firmwarePath = path;
    }

    std::vector<uint8_t> NfsConverter::buildZero(size_t length) {
        return std::vector<uint8_t>(length, 0);
    }

    Result<std::vector<uint8_t>> NfsConverter::getHeader(const std::filesystem::path &nfsFile) {
        std::ifstream file(nfsFile, std::ios::binary);
        if (!file) {
            return std::unexpected(ErrorCode::FileNotFound);
        }

        std::vector<uint8_t> header(HEADER_SIZE);
        file.read(reinterpret_cast<char *>(header.data()), HEADER_SIZE);
        
        if (!file || file.gcount() != HEADER_SIZE) {
            return std::unexpected(ErrorCode::IOError);
        }

        return header;
    }

    Result<void> NfsConverter::combineNfsFiles(const std::filesystem::path &outputFile,
                                                const std::filesystem::path &nfsDir) {
        std::ofstream output(outputFile, std::ios::binary);
        if (!output) {
            return std::unexpected(ErrorCode::IOError);
        }

        int fileIndex = 0;
        while (true) {
            char filename[64];
            std::snprintf(filename, sizeof(filename), "hif_%06d.nfs", fileIndex++);
            auto nfsPath = nfsDir / filename;

            if (!std::filesystem::exists(nfsPath)) {
                break;
            }

            std::ifstream input(nfsPath, std::ios::binary);
            if (!input) {
                return std::unexpected(ErrorCode::IOError);
            }

            // Copy file contents
            output << input.rdbuf();
        }

        return {};
    }

    Result<void> NfsConverter::splitNfsFile(const std::filesystem::path &inputFile,
                                             const std::filesystem::path &nfsDir) {
        std::ifstream input(inputFile, std::ios::binary);
        if (!input) {
            return std::unexpected(ErrorCode::FileNotFound);
        }

        // Get file size
        input.seekg(0, std::ios::end);
        size_t fileSize = input.tellg();
        input.seekg(0, std::ios::beg);

        int fileIndex = 0;
        size_t remaining = fileSize;

        while (remaining > 0) {
            char filename[64];
            std::snprintf(filename, sizeof(filename), "hif_%06d.nfs", fileIndex++);
            auto outputPath = nfsDir / filename;

            std::ofstream output(outputPath, std::ios::binary);
            if (!output) {
                return std::unexpected(ErrorCode::IOError);
            }

            size_t chunkSize = std::min(remaining, static_cast<size_t>(NFS_SIZE));
            std::vector<char> buffer(chunkSize);
            
            input.read(buffer.data(), chunkSize);
            output.write(buffer.data(), chunkSize);

            remaining -= chunkSize;
        }

        return {};
    }

    Result<void> NfsConverter::encryptDecryptNfs(const std::filesystem::path &inputFile,
                                                  const std::filesystem::path &outputFile,
                                                  const std::vector<uint8_t> &key,
                                                  const std::vector<uint8_t> &iv,
                                                  bool encrypt,
                                                  const std::vector<uint8_t> &header) {
        std::ifstream input(inputFile, std::ios::binary);
        if (!input) {
            return std::unexpected(ErrorCode::FileNotFound);
        }

        std::ofstream output(outputFile, std::ios::binary);
        if (!output) {
            return std::unexpected(ErrorCode::IOError);
        }

        // Write header first
        output.write(reinterpret_cast<const char *>(header.data()), header.size());

        // Initialize AES context
        EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
        if (!ctx) {
            return std::unexpected(ErrorCode::EncryptionError);
        }

        int result;
        if (encrypt) {
            result = EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), nullptr, key.data(), iv.data());
        } else {
            result = EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), nullptr, key.data(), iv.data());
        }

        if (result != 1) {
            EVP_CIPHER_CTX_free(ctx);
            return std::unexpected(ErrorCode::EncryptionError);
        }

        // Process file in chunks
        constexpr size_t bufferSize = SECTOR_SIZE;
        std::vector<uint8_t> inputBuffer(bufferSize);
        std::vector<uint8_t> outputBuffer(bufferSize + EVP_CIPHER_block_size(EVP_aes_128_cbc()));

        while (input.read(reinterpret_cast<char *>(inputBuffer.data()), bufferSize) || input.gcount() > 0) {
            int bytesRead = input.gcount();
            int outputLength = 0;

            if (encrypt) {
                result = EVP_EncryptUpdate(ctx,
                                            outputBuffer.data(),
                                            &outputLength,
                                            inputBuffer.data(),
                                            bytesRead);
            } else {
                result = EVP_DecryptUpdate(ctx,
                                            outputBuffer.data(),
                                            &outputLength,
                                            inputBuffer.data(),
                                            bytesRead);
            }

            if (result != 1) {
                EVP_CIPHER_CTX_free(ctx);
                return std::unexpected(ErrorCode::EncryptionError);
            }

            output.write(reinterpret_cast<const char *>(outputBuffer.data()), outputLength);
        }

        // Finalize
        int finalLength = 0;
        if (encrypt) {
            result = EVP_EncryptFinal_ex(ctx, outputBuffer.data(), &finalLength);
        } else {
            result = EVP_DecryptFinal_ex(ctx, outputBuffer.data(), &finalLength);
        }

        if (result == 1 && finalLength > 0) {
            output.write(reinterpret_cast<const char *>(outputBuffer.data()), finalLength);
        }

        EVP_CIPHER_CTX_free(ctx);
        return {};
    }

    Result<void> NfsConverter::unpackNfs(const std::filesystem::path &inputFile,
                                          const std::filesystem::path &outputFile,
                                          const std::vector<uint8_t> &header) {
        std::ifstream input(inputFile, std::ios::binary);
        if (!input) {
            return std::unexpected(ErrorCode::FileNotFound);
        }

        std::ofstream output(outputFile, std::ios::binary);
        if (!output) {
            return std::unexpected(ErrorCode::IOError);
        }

        // Skip header
        input.seekg(HEADER_SIZE);

        // Process sectors
        std::vector<char> buffer(SECTOR_SIZE);
        while (input.read(buffer.data(), SECTOR_SIZE) || input.gcount() > 0) {
            size_t bytesRead = input.gcount();
            // Write data part (skip sector header if present)
            output.write(buffer.data(), bytesRead);
        }

        return {};
    }

    Result<std::vector<uint8_t>> NfsConverter::packNfs(
        const std::filesystem::path &inputFile,
        const std::filesystem::path &outputFile,
        const std::array<int64_t, 2> &sizes) {
        
        std::ifstream input(inputFile, std::ios::binary);
        if (!input) {
            return std::unexpected(ErrorCode::FileNotFound);
        }

        std::ofstream output(outputFile, std::ios::binary);
        if (!output) {
            return std::unexpected(ErrorCode::IOError);
        }

        // Create header
        std::vector<uint8_t> header(HEADER_SIZE, 0);
        // TODO: Populate header based on sizes
        
        // Write header
        output.write(reinterpret_cast<const char *>(header.data()), header.size());

        // Copy and pack data
        std::vector<char> buffer(SECTOR_SIZE);
        while (input.read(buffer.data(), SECTOR_SIZE) || input.gcount() > 0) {
            size_t bytesRead = input.gcount();
            output.write(buffer.data(), bytesRead);
            
            // Pad to sector size if needed
            if (bytesRead < SECTOR_SIZE) {
                std::vector<char> padding(SECTOR_SIZE - bytesRead, 0);
                output.write(padding.data(), padding.size());
            }
        }

        return header;
    }

    Result<std::array<int64_t, 2>> NfsConverter::manipulateIso(
        const std::filesystem::path &inputFile,
        const std::filesystem::path &outputFile,
        bool decrypt) {
        
        std::ifstream input(inputFile, std::ios::binary);
        if (!input) {
            return std::unexpected(ErrorCode::FileNotFound);
        }

        std::ofstream output(outputFile, std::ios::binary);
        if (!output) {
            return std::unexpected(ErrorCode::IOError);
        }

        // Get file sizes
        input.seekg(0, std::ios::end);
        int64_t inputSize = input.tellg();
        input.seekg(0, std::ios::beg);

        // Simple copy for now - full implementation would handle disc format specifics
        output << input.rdbuf();

        output.flush();
        int64_t outputSize = output.tellp();

        return std::array<int64_t, 2>{inputSize, outputSize};
    }

    Result<void> NfsConverter::patchFakeSigning(const std::filesystem::path &fwFile) {
        // Simplified implementation - would patch firmware to allow fake signing
        if (!std::filesystem::exists(fwFile)) {
            return std::unexpected(ErrorCode::FileNotFound);
        }

        // TODO: Implement actual patching logic
        return {};
    }

    Result<void> NfsConverter::isoToNfs(const std::filesystem::path &isoPath,
                                         const std::filesystem::path &nfsDir,
                                         const std::vector<uint8_t> &encryptionKey,
                                         bool keepIntermediateFiles,
                                         bool keepLegitSignature) {
        
        if (!std::filesystem::exists(isoPath)) {
            return std::unexpected(ErrorCode::FileNotFound);
        }

        // Create NFS directory
        std::filesystem::create_directories(nfsDir);

        // Temporary files
        auto tempDir = std::filesystem::temp_directory_path();
        auto hifUnpack = tempDir / "hif_unpack.nfs";
        auto hifDec = tempDir / "hif_dec.nfs";
        auto hif = tempDir / "hif.nfs";

        // Patch firmware if needed
        if (!keepLegitSignature && !firmwarePath.empty()) {
            auto patchResult = patchFakeSigning(firmwarePath);
            if (!patchResult) {
                return patchResult;
            }
        }

        // Manipulate ISO
        auto sizeResult = manipulateIso(isoPath, hifUnpack, false);
        if (!sizeResult) {
            return std::unexpected(sizeResult.error());
        }

        // Pack NFS
        auto headerResult = packNfs(hifUnpack, hifDec, *sizeResult);
        if (!headerResult) {
            return std::unexpected(headerResult.error());
        }

        if (!keepIntermediateFiles) {
            std::filesystem::remove(hifUnpack);
        }

        // Encrypt
        auto iv = buildZero(encryptionKey.size());
        auto encryptResult =
            encryptDecryptNfs(hifDec, hif, encryptionKey, iv, true, *headerResult);
        if (!encryptResult) {
            return encryptResult;
        }

        if (!keepIntermediateFiles) {
            std::filesystem::remove(hifDec);
        }

        // Split into NFS files
        auto splitResult = splitNfsFile(hif, nfsDir);
        if (!splitResult) {
            return splitResult;
        }

        if (!keepIntermediateFiles) {
            std::filesystem::remove(hif);
        }

        return {};
    }

    Result<void> NfsConverter::nfsToIso(const std::filesystem::path &nfsDir,
                                         const std::filesystem::path &isoPath,
                                         const std::vector<uint8_t> &decryptionKey,
                                         bool keepIntermediateFiles) {
        
        auto nfsFile = nfsDir / "hif_000000.nfs";
        if (!std::filesystem::exists(nfsFile)) {
            return std::unexpected(ErrorCode::FileNotFound);
        }

        // Temporary files
        auto tempDir = std::filesystem::temp_directory_path();
        auto hif = tempDir / "hif.nfs";
        auto hifDec = tempDir / "hif_dec.nfs";
        auto hifUnpack = tempDir / "hif_unpack.nfs";

        // Get header
        auto headerResult = getHeader(nfsFile);
        if (!headerResult) {
            return std::unexpected(headerResult.error());
        }

        // Combine NFS files
        auto combineResult = combineNfsFiles(hif, nfsDir);
        if (!combineResult) {
            return combineResult;
        }

        // Decrypt
        auto iv = buildZero(decryptionKey.size());
        auto decryptResult =
            encryptDecryptNfs(hif, hifDec, decryptionKey, iv, false, *headerResult);
        if (!decryptResult) {
            return decryptResult;
        }

        if (!keepIntermediateFiles) {
            std::filesystem::remove(hif);
        }

        // Unpack
        auto unpackResult = unpackNfs(hifDec, hifUnpack, *headerResult);
        if (!unpackResult) {
            return unpackResult;
        }

        if (!keepIntermediateFiles) {
            std::filesystem::remove(hifDec);
        }

        // Manipulate to ISO
        auto manipulateResult = manipulateIso(hifUnpack, isoPath, true);
        if (!manipulateResult) {
            return std::unexpected(manipulateResult.error());
        }

        if (!keepIntermediateFiles) {
            std::filesystem::remove(hifUnpack);
        }

        return {};
    }

} // namespace wiivc::nfsconvert
