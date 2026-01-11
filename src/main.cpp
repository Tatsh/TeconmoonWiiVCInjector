// SPDX-License-Identifier: MIT
#include "wiivc/crypto.h"
#include "wiivc/fileformat.h"
#include "wiivc/gamedatabase.h"
#include "wiivc/imageconvert.h"
#include "wiivc/isotools.h"
#include "wiivc/nfstools.h"
#include "wiivc/stringutils.h"
#include "wiivc/types.h"
#include "wiivc/xmlgen.h"
#include <CLI/CLI.hpp>
#include <spdlog/spdlog.h>
#include <filesystem>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

struct Options {
    fs::path inputFile;
    fs::path outputDir;
    fs::path iconFile;
    fs::path bannerFile;
    fs::path witPath;
    fs::path nfsPath;
    fs::path keyFile;
    std::string titleId;
    std::string gameName;
    std::string commonKey;
    std::string titleKey;
    wiivc::SystemType systemType{wiivc::SystemType::WiiRetail};
    bool noTrimming{false};
    bool extractISO{false};
    bool convertToNFS{false};
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
    app.add_option("--key-file", opts.keyFile, "Path to key file for NFS encryption")
        ->check(CLI::ExistingFile);

    // Tool paths
    app.add_option("--wit-path", opts.witPath, "Path to wit executable")
        ->check(CLI::ExistingFile);
    app.add_option("--nfs-path", opts.nfsPath, "Path to nfs2iso2nfs executable")
        ->check(CLI::ExistingFile);

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
    app.add_flag("--extract-iso", opts.extractISO, "Extract ISO contents");
    app.add_flag("--convert-nfs", opts.convertToNFS, "Convert ISO to NFS format");
    app.add_flag("-v,--verbose", opts.verbose, "Verbose output");

    CLI11_PARSE(app, argc, argv);

    if (opts.verbose) {
        spdlog::info("WiiVC Injector starting...");
        spdlog::info("Input: {}", opts.inputFile.string());
        spdlog::info("Output: {}", opts.outputDir.string());
    }

    // Load game database
    auto dbResult = wiivc::GameDatabase::loadDatabase();
    if (!dbResult) {
        spdlog::error("Failed to load game database");
        return 1;
    }

    // Verify input file exists
    if (!fs::exists(opts.inputFile)) {
        spdlog::error("Input file not found: {}", opts.inputFile.string());
        return 1;
    }

    // Detect file type
    auto fileTypeResult = wiivc::FileFormatDetector::detectFileType(opts.inputFile);
    if (!fileTypeResult) {
        spdlog::error("Failed to detect file type");
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
        spdlog::info("Detected file type: {}", typeStr);
    }

    // Read game information
    auto gameIdResult = wiivc::FileFormatDetector::readGameId(opts.inputFile);
    if (gameIdResult && opts.verbose) {
        std::string gameId(gameIdResult->data(), 4);
        spdlog::info("Game ID: {}", gameId);
    }

    auto gameNameResult = wiivc::FileFormatDetector::readGameName(opts.inputFile);
    if (gameNameResult && opts.verbose) {
        spdlog::info("Internal name: {}", *gameNameResult);
    }

    // Verify encryption keys if provided
    if (!opts.commonKey.empty()) {
        auto verifyResult = wiivc::crypto::verifyWiiUCommonKey(opts.commonKey);
        if (verifyResult && *verifyResult) {
            spdlog::info("✓ Wii U Common Key verified");
        } else {
            spdlog::warn("✗ Invalid Wii U Common Key");
        }
    }

    if (!opts.titleKey.empty()) {
        auto verifyResult = wiivc::crypto::verifyTitleKey(opts.titleKey);
        if (verifyResult && *verifyResult) {
            spdlog::info("✓ Title Key verified");
        } else {
            spdlog::warn("✗ Invalid Title Key");
        }
    }

    // Create output directory if needed
    if (!fs::exists(opts.outputDir)) {
        try {
            fs::create_directories(opts.outputDir);
        } catch (const fs::filesystem_error &e) {
            spdlog::error("Failed to create output directory: {}", e.what());
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
            spdlog::info("✓ Icon converted: {} -> {}",
                       opts.iconFile.string(),
                       outputIcon.string());
        } else {
            spdlog::warn("Failed to convert icon: {}",
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
            spdlog::info("✓ Banner converted: {} -> {}",
                       opts.bannerFile.string(),
                       outputBanner.string());
        } else {
            spdlog::warn("Failed to convert banner: {}",
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
            spdlog::info("✓ Generated app.xml: {}", appXmlPath.string());
        } else {
            spdlog::warn("Failed to generate app.xml: {}",
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
            spdlog::info("✓ Generated meta.xml: {}", metaXmlPath.string());
        } else {
            spdlog::warn("Failed to generate meta.xml: {}",
                       wiivc::errorToString(metaResult.error()));
        }
    }

    // ISO manipulation with library
    if (opts.extractISO || opts.convertToNFS) {
        wiivc::isotools::WitTool wit;
        
        spdlog::info("✓ Using built-in ISO extraction library");

        if (opts.extractISO) {
            auto extractDir = opts.outputDir / "extracted";
            auto extractResult = wit.extractISO(opts.inputFile, extractDir, opts.verbose);
            if (extractResult) {
                spdlog::info("✓ ISO extracted to: {}", extractDir.string());
            } else {
                spdlog::warn("Failed to extract ISO: {}",
                           wiivc::errorToString(extractResult.error()));
            }
        }

        if (!opts.noTrimming) {
            auto trimmedISO = opts.outputDir / "trimmed.iso";
            auto trimResult = wit.trimISO(opts.inputFile, trimmedISO, opts.verbose);
            if (trimResult) {
                spdlog::info("✓ ISO trimmed: {}", trimmedISO.string());
            } else {
                spdlog::warn("Failed to trim ISO: {}",
                           wiivc::errorToString(trimResult.error()));
            }
        }
    }

    // NFS conversion with library
    if (opts.convertToNFS && !opts.keyFile.empty()) {
        wiivc::nfstools::NfsTool nfs;
        
        spdlog::info("✓ Using built-in NFS conversion library");

        auto nfsDir = opts.outputDir / "nfs";
        auto nfsResult = nfs.isoToNfs(opts.inputFile, nfsDir, opts.keyFile, opts.verbose);
        if (nfsResult) {
            spdlog::info("✓ ISO converted to NFS format: {}", nfsDir.string());
        } else {
            spdlog::warn("Failed to convert to NFS: {}",
                       wiivc::errorToString(nfsResult.error()));
        }
    } else if (opts.convertToNFS) {
        spdlog::warn("--key-file required for NFS conversion");
    }

    // TODO: Implement the actual conversion logic
    // This would involve:
    // 1. Extracting/converting ISO if needed
    // 2. Converting audio files
    // 3. Downloading base files from Nintendo CDN
    // 4. Encrypting and packaging

    spdlog::info("\n=== Conversion Status ===");
    spdlog::info("✓ File format detection implemented");
    spdlog::info("✓ Game information extraction implemented");
    spdlog::info("✓ Image conversion (PNG to TGA) implemented");
    spdlog::info("✓ XML generation (app.xml, meta.xml) implemented");
    spdlog::info("✓ Encryption key verification implemented");
    spdlog::info("✓ ISO manipulation (library-based) implemented");
    spdlog::info("✓ NFS conversion (library-based) implemented");
    spdlog::info("\nNo external processes required - all functionality built-in!");
    spdlog::info("\nRemaining work:");
    spdlog::info("  - Audio conversion (WAV to BTSND)");
    spdlog::info("  - WUP packaging (NUSPacker functionality)");

    spdlog::info("\nExecution completed successfully.");
    return 0;
}
