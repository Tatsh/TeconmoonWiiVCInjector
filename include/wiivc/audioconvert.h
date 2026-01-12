// Copyright (c) 2025 - Wii VC Injector C++ Port
// SPDX-License-Identifier: MIT
#pragma once

#include "types.h"
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <vector>

namespace wiivc::audioconvert {

// Convert WAV file to BTSND format
// BTSND is a simple PCM wrapper format used by Wii U for menu sounds
[[nodiscard]] Result<void> wavToBtsnd(const std::filesystem::path &wavPath,
                                       const std::filesystem::path &btsndPath);

} // namespace wiivc::audioconvert
