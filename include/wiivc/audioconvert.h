// Copyright (c) 2025 - Wii VC Injector C++ Port
// SPDX-License-Identifier: MIT
#pragma once

#include "types.h"
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <vector>

namespace wiivc::audioconvert {

// BTSND audio format constants
constexpr uint32_t BTSND_MAGIC = 0x42545344; // 'BTSD'
constexpr uint32_t BTSND_VERSION = 0x00010000;

// WAV format structures
struct WavHeader {
    char riffid[4];      // 'RIFF'
    uint32_t filesize;   // File size - 8
    char waveid[4];      // 'WAVE'
    char fmtid[4];       // 'fmt '
    uint32_t fmtsize;    // Format chunk size
    uint16_t format;     // Audio format (1 = PCM)
    uint16_t channels;   // Number of channels
    uint32_t samplerate; // Sample rate
    uint32_t byterate;   // Byte rate
    uint16_t blockalign; // Block align
    uint16_t bitspersample; // Bits per sample
    char dataid[4];      // 'data'
    uint32_t datasize;   // Data size
};

// BTSND format structures
struct BtsndHeader {
    uint32_t magic;      // 'BTSD'
    uint32_t version;    // Version
    uint32_t samplerate; // Sample rate
    uint32_t channels;   // Number of channels (1 or 2)
    uint32_t loopstart;  // Loop start sample
    uint32_t datasize;   // Audio data size in bytes
    uint32_t reserved[2]; // Reserved
};

// Convert WAV file to BTSND format
// BTSND is a simple PCM wrapper format used by Wii U for menu sounds
[[nodiscard]] Result<void> wavToBtsnd(const std::filesystem::path &wavPath,
                                       const std::filesystem::path &btsndPath);

// Read WAV file header
[[nodiscard]] Result<WavHeader> readWavHeader(const std::filesystem::path &path);

// Convert PCM samples (handles endianness)
[[nodiscard]] Result<std::vector<uint8_t>>
convertPCMSamples(const std::vector<uint8_t> &wavData, const WavHeader &header);

} // namespace wiivc::audioconvert
