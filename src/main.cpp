// SPDX-License-Identifier: MIT
#include "wiivc/fileformat.h"
#include "wiivc/gamedatabase.h"
#include "wiivc/stringutils.h"
#include "wiivc/types.h"
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

    // Create output directory if needed
    if (!fs::exists(opts.outputDir)) {
        try {
            fs::create_directories(opts.outputDir);
        } catch (const fs::filesystem_error &e) {
            fmt::print(stderr, "Error: Failed to create output directory: {}\n", e.what());
            return 1;
        }
    }

    // TODO: Implement the actual conversion logic
    // This would involve:
    // 1. Converting images to required formats
    // 2. Building the injection package
    // 3. Encrypting with provided keys
    // 4. Creating output package

    fmt::print("\n=== Conversion Status ===\n");
    fmt::print("Note: Full conversion logic not yet implemented.\n");
    fmt::print("This would require integrating or porting tools like:\n");
    fmt::print("  - wit (Wiimms ISO Tools)\n");
    fmt::print("  - nfs2iso2nfs\n");
    fmt::print("  - JNUSTool\n");
    fmt::print("  - NUSPacker\n");
    fmt::print("  - Image conversion libraries\n");
    fmt::print("  - Audio conversion libraries\n");

    fmt::print("\nPlaceholder execution completed successfully.\n");
    return 0;
}
