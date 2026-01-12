// SPDX-License-Identifier: MIT
#include "../internal/download.h"
#include "internal/crypto.h"
#include <curl/curl.h>
#include <fstream>

namespace wiivc::download {

    namespace {
        // CURL write callback for file download
        size_t writeToFile(void *ptr, size_t size, size_t nmemb, void *stream) {
            auto *file = static_cast<std::ofstream *>(stream);
            file->write(static_cast<const char *>(ptr), size * nmemb);
            return size * nmemb;
        }

        // CURL write callback for memory download
        size_t writeToMemory(void *ptr, size_t size, size_t nmemb, void *userdata) {
            auto *buffer = static_cast<std::vector<uint8_t> *>(userdata);
            size_t totalSize = size * nmemb;
            auto *data = static_cast<const uint8_t *>(ptr);
            buffer->insert(buffer->end(), data, data + totalSize);
            return totalSize;
        }

        // Progress callback wrapper
        struct ProgressData {
            ProgressCallback callback;
        };

        int progressCallback(void *clientp,
                              curl_off_t dltotal,
                              curl_off_t dlnow,
                              curl_off_t ultotal,
                              curl_off_t ulnow) {
            (void)ultotal;
            (void)ulnow;
            auto *data = static_cast<ProgressData *>(clientp);
            if (data->callback) {
                if (!data->callback(static_cast<size_t>(dlnow), static_cast<size_t>(dltotal))) {
                    return 1; // Cancel download
                }
            }
            return 0;
        }

        // Initialize curl global state (called once)
        struct CurlInitializer {
            CurlInitializer() { curl_global_init(CURL_GLOBAL_DEFAULT); }
            ~CurlInitializer() { curl_global_cleanup(); }
        };

        CurlInitializer curlInit;
    } // namespace

    Result<void> downloadFile(std::string_view url,
                               const std::filesystem::path &outputPath,
                               ProgressCallback progress) {
        CURL *curl = curl_easy_init();
        if (!curl) {
            return std::unexpected(ErrorCode::NetworkError);
        }

        std::ofstream outFile(outputPath, std::ios::binary);
        if (!outFile) {
            curl_easy_cleanup(curl);
            return std::unexpected(ErrorCode::IOError);
        }

        curl_easy_setopt(curl, CURLOPT_URL, url.data());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToFile);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &outFile);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);

        ProgressData progressData{progress};
        if (progress) {
            curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progressCallback);
            curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &progressData);
            curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        }

        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        outFile.close();

        if (res != CURLE_OK) {
            std::filesystem::remove(outputPath);
            return std::unexpected(ErrorCode::NetworkError);
        }

        return {};
    }

    Result<std::vector<uint8_t>> downloadToMemory(std::string_view url, ProgressCallback progress) {
        CURL *curl = curl_easy_init();
        if (!curl) {
            return std::unexpected(ErrorCode::NetworkError);
        }

        std::vector<uint8_t> buffer;

        curl_easy_setopt(curl, CURLOPT_URL, url.data());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToMemory);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);

        ProgressData progressData{progress};
        if (progress) {
            curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progressCallback);
            curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &progressData);
            curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        }

        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            return std::unexpected(ErrorCode::NetworkError);
        }

        return buffer;
    }

    Result<bool> urlExists(std::string_view url) {
        CURL *curl = curl_easy_init();
        if (!curl) {
            return std::unexpected(ErrorCode::NetworkError);
        }

        curl_easy_setopt(curl, CURLOPT_URL, url.data());
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        CURLcode res = curl_easy_perform(curl);
        
        long responseCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            return std::unexpected(ErrorCode::NetworkError);
        }

        return responseCode == 200;
    }

    Result<size_t> getRemoteFileSize(std::string_view url) {
        CURL *curl = curl_easy_init();
        if (!curl) {
            return std::unexpected(ErrorCode::NetworkError);
        }

        curl_easy_setopt(curl, CURLOPT_URL, url.data());
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);

        CURLcode res = curl_easy_perform(curl);
        
        curl_off_t fileSize = 0;
        curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &fileSize);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK || fileSize < 0) {
            return std::unexpected(ErrorCode::NetworkError);
        }

        return static_cast<size_t>(fileSize);
    }

    Result<void> BaseFileDownloader::downloadBaseFiles(ProgressCallback progress) {
        if (cacheDir.empty()) {
            return std::unexpected(ErrorCode::InvalidFile);
        }

        // Create cache directory
        if (!std::filesystem::exists(cacheDir)) {
            try {
                std::filesystem::create_directories(cacheDir);
            } catch (...) {
                return std::unexpected(ErrorCode::IOError);
            }
        }

        // Base files to download from Nintendo CDN
        // These are example URLs - actual implementation would need real Nintendo CDN URLs
        struct FileToDownload {
            std::string url;
            std::filesystem::path localPath;
            std::string expectedHash;
        };

        // Note: These are placeholder URLs and hashes
        // Real implementation needs actual Nintendo CDN endpoints
        std::vector<FileToDownload> files = {
            {"http://ccs.cdn.wup.shop.nintendo.net/ccs/download/0005001010004000/deint.txt",
             cacheDir / "deint.txt",
             "E707A62EE5491DD16E5494631EA9870A"},
            // Add more base files as needed
        };

        for (const auto &file : files) {
            // Check if file already exists with correct hash
            if (std::filesystem::exists(file.localPath)) {
                auto hashResult = crypto::computeMD5(file.localPath);
                if (hashResult && crypto::verifyMD5(file.localPath, file.expectedHash).value_or(false)) {
                    continue; // File already cached
                }
            }

            // Download the file
            auto downloadResult = downloadFile(file.url, file.localPath, progress);
            if (!downloadResult) {
                return downloadResult;
            }

            // Verify hash
            if (!crypto::verifyMD5(file.localPath, file.expectedHash).value_or(false)) {
                std::filesystem::remove(file.localPath);
                return std::unexpected(ErrorCode::HashMismatch);
            }
        }

        return {};
    }

    Result<bool> BaseFileDownloader::areBaseFilesCached() {
        if (cacheDir.empty() || !std::filesystem::exists(cacheDir)) {
            return false;
        }

        // Check if all required base files exist
        // This is a simplified check - real implementation would check all files
        std::vector<std::filesystem::path> requiredFiles = {
            cacheDir / "deint.txt",
            // Add more required files
        };

        for (const auto &file : requiredFiles) {
            if (!std::filesystem::exists(file)) {
                return false;
            }
        }

        return true;
    }

} // namespace wiivc::download
