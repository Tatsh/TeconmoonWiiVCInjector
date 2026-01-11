// SPDX-License-Identifier: MIT
#pragma once

#include "types.h"
#include <cstdint>
#include <filesystem>
#include <vector>

namespace wiivc::image {

    // Image data structure
    struct ImageData {
        std::vector<uint8_t> pixels;
        uint32_t width{0};
        uint32_t height{0};
        uint32_t channels{0};
    };

    // Load image from file (supports PNG, JPG, TGA, BMP, etc.)
    [[nodiscard]] Result<ImageData> loadImage(const std::filesystem::path &path);

    // Save image as PNG
    [[nodiscard]] Result<void> savePNG(const std::filesystem::path &path,
                                        const ImageData &image);

    // Save image as TGA
    [[nodiscard]] Result<void> saveTGA(const std::filesystem::path &path,
                                        const ImageData &image,
                                        uint32_t bitsPerPixel,
                                        bool compress);

    // Resize image to specific dimensions
    [[nodiscard]] Result<ImageData> resizeImage(const ImageData &image,
                                                 uint32_t newWidth,
                                                 uint32_t newHeight);

    // Convert image to specific channel count (e.g., RGB to RGBA)
    [[nodiscard]] Result<ImageData> convertChannels(const ImageData &image,
                                                     uint32_t targetChannels);

    // Convert PNG to TGA with specific format
    [[nodiscard]] Result<void> convertPNGToTGA(const std::filesystem::path &inputPNG,
                                                const std::filesystem::path &outputTGA,
                                                uint32_t targetWidth,
                                                uint32_t targetHeight,
                                                uint32_t bitsPerPixel,
                                                bool compress);

} // namespace wiivc::image
