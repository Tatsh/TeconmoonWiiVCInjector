// SPDX-License-Identifier: MIT
#include "../internal/imageconvert.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_resize2.h>
#include <stb_image_write.h>

namespace wiivc::image {

    Result<ImageData> loadImage(const std::filesystem::path &path) {
        if (!std::filesystem::exists(path)) {
            return std::unexpected(ErrorCode::FileNotFound);
        }

        int width = 0;
        int height = 0;
        int channels = 0;

        unsigned char *data =
            stbi_load(path.string().c_str(), &width, &height, &channels, 0);

        if (!data) {
            return std::unexpected(ErrorCode::InvalidFile);
        }

        ImageData result;
        result.width = static_cast<uint32_t>(width);
        result.height = static_cast<uint32_t>(height);
        result.channels = static_cast<uint32_t>(channels);

        size_t dataSize = static_cast<size_t>(width) * height * channels;
        result.pixels.assign(data, data + dataSize);

        stbi_image_free(data);
        return result;
    }

    Result<void> savePNG(const std::filesystem::path &path, const ImageData &image) {
        if (image.pixels.empty()) {
            return std::unexpected(ErrorCode::InvalidFile);
        }

        int result = stbi_write_png(path.string().c_str(),
                                     static_cast<int>(image.width),
                                     static_cast<int>(image.height),
                                     static_cast<int>(image.channels),
                                     image.pixels.data(),
                                     static_cast<int>(image.width * image.channels));

        if (result == 0) {
            return std::unexpected(ErrorCode::IOError);
        }

        return {};
    }

    Result<void> saveTGA(const std::filesystem::path &path,
                         const ImageData &image,
                         uint32_t bitsPerPixel,
                         bool compress) {
        if (image.pixels.empty()) {
            return std::unexpected(ErrorCode::InvalidFile);
        }

        // Validate bits per pixel matches channels
        uint32_t expectedChannels = bitsPerPixel / 8;
        if (image.channels != expectedChannels) {
            return std::unexpected(ErrorCode::InvalidFormat);
        }

        // stb_image_write doesn't support compression control for TGA
        // It always writes uncompressed TGA
        int result = stbi_write_tga(path.string().c_str(),
                                     static_cast<int>(image.width),
                                     static_cast<int>(image.height),
                                     static_cast<int>(image.channels),
                                     image.pixels.data());

        if (result == 0) {
            return std::unexpected(ErrorCode::IOError);
        }

        return {};
    }

    Result<ImageData> resizeImage(const ImageData &image,
                                   uint32_t newWidth,
                                   uint32_t newHeight) {
        if (image.pixels.empty()) {
            return std::unexpected(ErrorCode::InvalidFile);
        }

        ImageData result;
        result.width = newWidth;
        result.height = newHeight;
        result.channels = image.channels;

        size_t newSize = static_cast<size_t>(newWidth) * newHeight * image.channels;
        result.pixels.resize(newSize);

        int resizeResult = stbir_resize_uint8_linear(
            image.pixels.data(),
            static_cast<int>(image.width),
            static_cast<int>(image.height),
            0,
            result.pixels.data(),
            static_cast<int>(newWidth),
            static_cast<int>(newHeight),
            0,
            static_cast<stbir_pixel_layout>(image.channels));

        if (resizeResult == 0) {
            return std::unexpected(ErrorCode::ConversionError);
        }

        return result;
    }

    Result<ImageData> convertChannels(const ImageData &image, uint32_t targetChannels) {
        if (image.pixels.empty()) {
            return std::unexpected(ErrorCode::InvalidFile);
        }

        if (image.channels == targetChannels) {
            return image; // No conversion needed
        }

        ImageData result;
        result.width = image.width;
        result.height = image.height;
        result.channels = targetChannels;

        size_t pixelCount = static_cast<size_t>(image.width) * image.height;
        result.pixels.resize(pixelCount * targetChannels);

        // Simple channel conversion
        for (size_t i = 0; i < pixelCount; ++i) {
            size_t srcIdx = i * image.channels;
            size_t dstIdx = i * targetChannels;

            if (targetChannels >= 1 && image.channels >= 1) {
                result.pixels[dstIdx] = image.pixels[srcIdx]; // R
            }
            if (targetChannels >= 2 && image.channels >= 2) {
                result.pixels[dstIdx + 1] = image.pixels[srcIdx + 1]; // G
            }
            if (targetChannels >= 3 && image.channels >= 3) {
                result.pixels[dstIdx + 2] = image.pixels[srcIdx + 2]; // B
            }
            if (targetChannels >= 4) {
                result.pixels[dstIdx + 3] =
                    image.channels >= 4 ? image.pixels[srcIdx + 3] : 255; // A
            }
        }

        return result;
    }

    Result<void> convertPNGToTGA(const std::filesystem::path &inputPNG,
                                  const std::filesystem::path &outputTGA,
                                  uint32_t targetWidth,
                                  uint32_t targetHeight,
                                  uint32_t bitsPerPixel,
                                  bool compress) {
        // Load the PNG
        auto imageResult = loadImage(inputPNG);
        if (!imageResult) {
            return std::unexpected(imageResult.error());
        }

        ImageData image = *imageResult;

        // Resize if needed
        if (image.width != targetWidth || image.height != targetHeight) {
            auto resizeResult = resizeImage(image, targetWidth, targetHeight);
            if (!resizeResult) {
                return std::unexpected(resizeResult.error());
            }
            image = *resizeResult;
        }

        // Convert channels if needed
        uint32_t targetChannels = bitsPerPixel / 8;
        if (image.channels != targetChannels) {
            auto convertResult = convertChannels(image, targetChannels);
            if (!convertResult) {
                return std::unexpected(convertResult.error());
            }
            image = *convertResult;
        }

        // Save as TGA
        return saveTGA(outputTGA, image, bitsPerPixel, compress);
    }

} // namespace wiivc::image
