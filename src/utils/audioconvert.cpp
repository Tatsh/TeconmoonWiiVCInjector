// Copyright (c) 2025 - Wii VC Injector C++ Port
// SPDX-License-Identifier: MIT
#include "wiivc/audioconvert.h"
#include <bit>
#include <cstring>
#include <fstream>

namespace wiivc::audioconvert {

Result<WavHeader> readWavHeader(const std::filesystem::path &path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return std::unexpected(ErrorCode::FileNotFound);
    }

    WavHeader header{};
    file.read(reinterpret_cast<char *>(&header), sizeof(WavHeader));
    if (!file) {
        return std::unexpected(ErrorCode::IOError);
    }

    // Verify RIFF header
    if (std::memcmp(header.riffid, "RIFF", 4) != 0 ||
        std::memcmp(header.waveid, "WAVE", 4) != 0) {
        return std::unexpected(ErrorCode::InvalidFormat);
    }

    // Verify format
    if (header.format != 1) { // Must be PCM
        return std::unexpected(ErrorCode::InvalidFormat);
    }

    return header;
}

Result<std::vector<uint8_t>> convertPCMSamples(const std::vector<uint8_t> &wavData,
                                                const WavHeader &header) {
    std::vector<uint8_t> output;
    output.reserve(wavData.size());

    // BTSND uses big-endian 16-bit PCM samples
    if (header.bitspersample == 16) {
        // Convert little-endian WAV to big-endian BTSND
        for (size_t i = 0; i + 1 < wavData.size(); i += 2) {
            uint16_t sample = wavData[i] | (wavData[i + 1] << 8);
            if constexpr (std::endian::native == std::endian::little) {
                sample = std::byteswap(sample);
            }
            output.push_back(static_cast<uint8_t>(sample >> 8));
            output.push_back(static_cast<uint8_t>(sample & 0xFF));
        }
    } else if (header.bitspersample == 8) {
        // Convert 8-bit unsigned to 16-bit signed
        for (uint8_t byte : wavData) {
            int16_t sample = (static_cast<int16_t>(byte) - 128) << 8;
            if constexpr (std::endian::native == std::endian::little) {
                sample = std::byteswap(sample);
            }
            output.push_back(static_cast<uint8_t>(sample >> 8));
            output.push_back(static_cast<uint8_t>(sample & 0xFF));
        }
    } else {
        return std::unexpected(ErrorCode::InvalidFormat);
    }

    return output;
}

Result<void> wavToBtsnd(const std::filesystem::path &wavPath,
                         const std::filesystem::path &btsndPath) {
    // Read WAV header
    auto headerResult = readWavHeader(wavPath);
    if (!headerResult) {
        return std::unexpected(headerResult.error());
    }
    const auto &wavHeader = *headerResult;

    // Read WAV data
    std::ifstream wavFile(wavPath, std::ios::binary);
    if (!wavFile) {
        return std::unexpected(ErrorCode::FileNotFound);
    }

    // Skip to data section
    wavFile.seekg(sizeof(WavHeader), std::ios::beg);
    std::vector<uint8_t> wavData(wavHeader.datasize);
    wavFile.read(reinterpret_cast<char *>(wavData.data()), wavHeader.datasize);
    if (!wavFile) {
        return std::unexpected(ErrorCode::IOError);
    }

    // Convert samples
    auto samplesResult = convertPCMSamples(wavData, wavHeader);
    if (!samplesResult) {
        return std::unexpected(samplesResult.error());
    }
    const auto &convertedData = *samplesResult;

    // Create BTSND header
    BtsndHeader btsndHeader{};
    btsndHeader.magic = BTSND_MAGIC;
    btsndHeader.version = BTSND_VERSION;
    btsndHeader.samplerate = wavHeader.samplerate;
    btsndHeader.channels = wavHeader.channels;
    btsndHeader.loopstart = 0; // No looping
    btsndHeader.datasize = static_cast<uint32_t>(convertedData.size());
    btsndHeader.reserved[0] = 0;
    btsndHeader.reserved[1] = 0;

    // Convert header to big-endian for file
    if constexpr (std::endian::native == std::endian::little) {
        btsndHeader.magic = std::byteswap(btsndHeader.magic);
        btsndHeader.version = std::byteswap(btsndHeader.version);
        btsndHeader.samplerate = std::byteswap(btsndHeader.samplerate);
        btsndHeader.channels = std::byteswap(btsndHeader.channels);
        btsndHeader.loopstart = std::byteswap(btsndHeader.loopstart);
        btsndHeader.datasize = std::byteswap(btsndHeader.datasize);
    }

    // Write BTSND file
    std::ofstream outFile(btsndPath, std::ios::binary);
    if (!outFile) {
        return std::unexpected(ErrorCode::IOError);
    }

    outFile.write(reinterpret_cast<const char *>(&btsndHeader), sizeof(BtsndHeader));
    outFile.write(reinterpret_cast<const char *>(convertedData.data()),
                  convertedData.size());

    if (!outFile) {
        return std::unexpected(ErrorCode::IOError);
    }

    return {};
}

} // namespace wiivc::audioconvert
