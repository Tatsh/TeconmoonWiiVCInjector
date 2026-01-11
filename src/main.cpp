// SPDX-License-Identifier: MIT
#include "wiivc/crypto.h"
#include "wiivc/fileformat.h"
#include "wiivc/gamedatabase.h"
#include "wiivc/imageconvert.h"
#include "wiivc/stringutils.h"
#include "wiivc/types.h"
#include "wiivc/xmlgen.h"
#include <CLI/CLI.hpp>
#include <fmt/core.h>
#include <filesystem>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

struct Options {
    fs::path inputFile;
    fs::path outputDir;
    fs::path iconFile;
    fs::path bannerFile;
    std::string titleId;
    std::string gameName;
    std::string commonKey;
    std::string titleKey;
    wiivc::SystemType systemType{wiivc::SystemType::WiiRetail};
    bool noTrimming{false};
    bool verbose{false};
};

int main(int argc, char **argv) {
    CLI::App app{"Wii Virtual Console Injector - Convert Wii/GameCube games for Wii U VC"};

    Options opts;

    // Input/output options
    app.add_option("-i,--input", opts.inputFile, "Input game file (ISO, WBFS, NKIT, etc.)")
        ->required()
        ->check(CLI::ExistingFile);

    app.add_option("-o,--output", opts.outputDir, "Output directory")
        ->required()
        ->check(CLI::ExistingDirectory);

    // Image options
    app.add_option("--icon", opts.iconFile, "Icon image file (128x128)")
        ->check(CLI::ExistingFile);

    app.add_option("--banner", opts.bannerFile, "Banner image file (1280x720)")
        ->check(CLI::ExistingFile);

    // Title options
    app.add_option("--title-id", opts.titleId, "Title ID (4 characters)");
    app.add_option("--game-name", opts.gameName, "Game name");

    // Encryption keys
    app.add_option("--common-key", opts.commonKey, "Wii U Common Key (32 hex characters)");
    app.add_option("--title-key", opts.titleKey, "Title Key (32 hex characters)");

    // System type
    std::map<std::string, wiivc::SystemType> systemTypeMap{
        {"wii", wiivc::SystemType::WiiRetail},
        {"wii-homebrew", wiivc::SystemType::WiiHomebrew},
        {"wii-nand", wiivc::SystemType::WiiNAND},
        {"gamecube", wiivc::SystemType::GameCubeRetail}};

    app.add_option("--system-type",
                   opts.systemType,
                   "System type (wii, wii-homebrew, wii-nand, gamecube)")
        ->transform(CLI::CheckedTransformer(systemTypeMap, CLI::ignore_case));

    // Other options
    app.add_flag("--no-trimming", opts.noTrimming, "Disable ISO trimming");
    app.add_flag("-v,--verbose", opts.verbose, "Verbose output");

    CLI11_PARSE(app, argc, argv);

    if (opts.verbose) {
        fmt::print("WiiVC Injector starting...\n");
        fmt::print("Input: {}\n", opts.inputFile.string());
        fmt::print("Output: {}\n", opts.outputDir.string());
    }

    // Load game database
    auto dbResult = wiivc::GameDatabase::loadDatabase();
    if (!dbResult) {
        fmt::print(stderr, "Error: Failed to load game database\n");
        return 1;
    }

    // Verify input file exists
    if (!fs::exists(opts.inputFile)) {
        fmt::print(stderr, "Error: Input file not found: {}\n", opts.inputFile.string());
        return 1;
    }

    // Detect file type
    auto fileTypeResult = wiivc::FileFormatDetector::detectFileType(opts.inputFile);
    if (!fileTypeResult) {
        fmt::print(stderr, "Error: Failed to detect file type\n");
        return 1;
    }

    if (opts.verbose) {
        const char *typeStr = "Unknown";
        switch (*fileTypeResult) {
            case wiivc::FileType::ISO:
                typeStr = "ISO";
                break;
            case wiivc::FileType::WBFS:
                typeStr = "WBFS";
                break;
            case wiivc::FileType::NKIT:
                typeStr = "NKIT";
                break;
            case wiivc::FileType::NASOS:
                typeStr = "NASOS";
                break;
            case wiivc::FileType::DOL:
                typeStr = "DOL";
                break;
            case wiivc::FileType::GCM:
                typeStr = "GameCube";
                break;
            default:
                break;
        }
        fmt::print("Detected file type: {}\n", typeStr);
    }

    // Read game information
    auto gameIdResult = wiivc::FileFormatDetector::readGameId(opts.inputFile);
    if (gameIdResult && opts.verbose) {
        std::string gameId(gameIdResult->data(), 4);
        fmt::print("Game ID: {}\n", gameId);
    }

    auto gameNameResult = wiivc::FileFormatDetector::readGameName(opts.inputFile);
    if (gameNameResult && opts.verbose) {
        fmt::print("Internal name: {}\n", *gameNameResult);
    }

    // Verify encryption keys if provided
    if (!opts.commonKey.empty()) {
        auto verifyResult = wiivc::crypto::verifyWiiUCommonKey(opts.commonKey);
        if (verifyResult && *verifyResult) {
            fmt::print("✓ Wii U Common Key verified\n");
        } else {
            fmt::print(stderr, "✗ Warning: Invalid Wii U Common Key\n");
        }
    }

    if (!opts.titleKey.empty()) {
        auto verifyResult = wiivc::crypto::verifyTitleKey(opts.titleKey);
        if (verifyResult && *verifyResult) {
            fmt::print("✓ Title Key verified\n");
        } else {
            fmt::print(stderr, "✗ Warning: Invalid Title Key\n");
        }
    }

    // Create output directory if needed
    if (!fs::exists(opts.outputDir)) {
        try {
            fs::create_directories(opts.outputDir);
        } catch (const fs::filesystem_error &e) {
            fmt::print(stderr, "Error: Failed to create output directory: {}\n", e.what());
            return 1;
        }
    }

    // Process images if provided
    if (!opts.iconFile.empty()) {
        auto outputIcon = opts.outputDir / "iconTex.tga";
        auto convertResult = wiivc::image::convertPNGToTGA(opts.iconFile,
                                                            outputIcon,
                                                            128,
                                                            128,
                                                            32,
                                                            false);
        if (convertResult) {
            fmt::print("✓ Icon converted: {} -> {}\n",
                       opts.iconFile.string(),
                       outputIcon.string());
        } else {
            fmt::print(stderr,
                       "✗ Warning: Failed to convert icon: {}\n",
                       wiivc::errorToString(convertResult.error()));
        }
    }

    if (!opts.bannerFile.empty()) {
        auto outputBanner = opts.outputDir / "bootTvTex.tga";
        auto convertResult = wiivc::image::convertPNGToTGA(opts.bannerFile,
                                                            outputBanner,
                                                            1280,
                                                            720,
                                                            24,
                                                            false);
        if (convertResult) {
            fmt::print("✓ Banner converted: {} -> {}\n",
                       opts.bannerFile.string(),
                       outputBanner.string());
        } else {
            fmt::print(stderr,
                       "✗ Warning: Failed to convert banner: {}\n",
                       wiivc::errorToString(convertResult.error()));
        }
    }

    // Generate XML files if we have enough information
    if (gameIdResult && !opts.titleId.empty()) {
        std::string gameId(gameIdResult->data(), 4);
        
        // Generate app.xml
        wiivc::xmlgen::AppXMLConfig appConfig;
        appConfig.titleId = "00050002" + opts.titleId;
        appConfig.titleIdHex = opts.titleId;
        
        auto appXmlPath = opts.outputDir / "app.xml";
        auto appResult = wiivc::xmlgen::saveAppXML(appXmlPath, appConfig);
        if (appResult) {
            fmt::print("✓ Generated app.xml: {}\n", appXmlPath.string());
        } else {
            fmt::print(stderr,
                       "✗ Warning: Failed to generate app.xml: {}\n",
                       wiivc::errorToString(appResult.error()));
        }

        // Generate meta.xml
        wiivc::xmlgen::MetaXMLConfig metaConfig;
        metaConfig.titleId = "00050002" + opts.titleId;
        metaConfig.titleIdHex = opts.titleId;
        metaConfig.productCode = "WUP-N-" + gameId;
        
        if (!opts.gameName.empty()) {
            metaConfig.longName = opts.gameName;
            metaConfig.shortName = opts.gameName;
        } else if (gameNameResult) {
            metaConfig.longName = *gameNameResult;
            metaConfig.shortName = *gameNameResult;
        } else {
            metaConfig.longName = gameId;
            metaConfig.shortName = gameId;
        }

        metaConfig.drcUse = 1; // Default DRC usage
        
        auto metaXmlPath = opts.outputDir / "meta.xml";
        auto metaResult = wiivc::xmlgen::saveMetaXML(metaXmlPath, metaConfig);
        if (metaResult) {
            fmt::print("✓ Generated meta.xml: {}\n", metaXmlPath.string());
        } else {
            fmt::print(stderr,
                       "✗ Warning: Failed to generate meta.xml: {}\n",
                       wiivc::errorToString(metaResult.error()));
        }
    }

    // TODO: Implement the actual conversion logic
    // This would involve:
    // 1. Extracting/converting ISO if needed
    // 2. Converting audio files
    // 3. Downloading base files from Nintendo CDN
    // 4. Encrypting and packaging

    fmt::print("\n=== Conversion Status ===\n");
    fmt::print("✓ File format detection implemented\n");
    fmt::print("✓ Game information extraction implemented\n");
    fmt::print("✓ Image conversion (PNG to TGA) implemented\n");
    fmt::print("✓ XML generation (app.xml, meta.xml) implemented\n");
    fmt::print("✓ Encryption key verification implemented\n");
    fmt::print("\nRemaining work:\n");
    fmt::print("  - Audio conversion (WAV to BTSND)\n");
    fmt::print("  - ISO extraction/manipulation (wit integration)\n");
    fmt::print("  - NFS conversion (nfs2iso2nfs integration)\n");
    fmt::print("  - Base file download (JNUSTool functionality)\n");
    fmt::print("  - WUP packaging (NUSPacker functionality)\n");

    fmt::print("\nExecution completed successfully.\n");
    return 0;
}
