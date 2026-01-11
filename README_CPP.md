# Wii Virtual Console Injector

C++23 command-line tool for converting Wii and GameCube games into Wii U Virtual Console format.

This is a portable C++ conversion of the original C# Windows Forms application.

## Features

- Convert Wii retail games (ISO, WBFS, NKIT, NASOS formats)
- Convert Wii homebrew (DOL files)
- Convert GameCube retail games
- Support for vWii NAND title launching
- Cross-platform (Windows, macOS, Linux)
- No exceptions, error handling via std::expected

## Building

### Requirements

- CMake 3.31 or later
- C++23 compatible compiler (GCC 13+, Clang 16+, MSVC 2022+)
- vcpkg for dependency management

### Dependencies

All dependencies are managed via vcpkg:
- CLI11 - Command-line argument parsing
- fmt - String formatting
- pugixml - XML parsing
- curl - HTTP downloads
- openssl - Cryptography
- zlib - Compression

### Build Instructions

```bash
# Clone the repository
git clone https://github.com/Tatsh/TeconmoonWiiVCInjector.git
cd TeconmoonWiiVCInjector

# Configure with vcpkg
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake

# Build
cmake --build build

# Install (optional)
cmake --install build
```

### Platform-Specific Notes

#### Windows
Requires Visual Studio 2022 or later with C++23 support.

#### macOS
Requires Xcode 14+ or Clang 16+ from Homebrew/MacPorts.

#### Linux
Requires GCC 13+ or Clang 16+.

## Usage

```bash
# Basic usage
wiivc-injector -i game.iso -o output_dir --icon icon.png --banner banner.png \
    --common-key <KEY> --title-key <KEY>

# With options
wiivc-injector -i game.iso -o output_dir \
    --system-type wii \
    --icon icon.png \
    --banner banner.png \
    --title-id GAME \
    --game-name "My Game" \
    --common-key <32_HEX_CHARS> \
    --title-key <32_HEX_CHARS> \
    --no-trimming \
    --verbose
```

### Options

- `-i, --input <file>` - Input game file (required)
- `-o, --output <dir>` - Output directory (required)
- `--icon <file>` - Icon image file (128x128 PNG)
- `--banner <file>` - Banner image file (1280x720 PNG)
- `--title-id <id>` - Title ID (4 characters)
- `--game-name <name>` - Game name
- `--common-key <key>` - Wii U Common Key (32 hex characters)
- `--title-key <key>` - Title Key (32 hex characters)
- `--system-type <type>` - System type: wii, wii-homebrew, wii-nand, gamecube
- `--no-trimming` - Disable ISO trimming
- `-v, --verbose` - Verbose output

## External Tools Integration

The original C# application relied on several external tools. For the C++ version, these need to be either:

1. **Integrated as libraries** (if available)
2. **Ported to C++** (if open source and feasible)
3. **Called as external processes** (as fallback)

### Tools Used

- **wit** (Wiimms ISO Tools) - ISO manipulation
- **nfs2iso2nfs** - NFS conversion
- **JNUSTool** - Nintendo CDN downloads
- **NUSPacker** - WUP package creation
- **Image conversion tools** - PNG/TGA conversion
- **Audio conversion tools** - WAV to BTSND conversion

## Project Structure

```
.
├── CMakeLists.txt          # Main CMake configuration
├── vcpkg.json              # Dependency manifest
├── .clang-format           # Code formatting rules
├── include/
│   └── wiivc/
│       ├── types.h         # Type definitions
│       ├── stringutils.h   # String utilities
│       └── gamedatabase.h  # Game database API
├── src/
│   ├── CMakeLists.txt      # Source CMake configuration
│   ├── main.cpp            # CLI entry point
│   └── utils/
│       └── gamedatabase.cpp # Game database implementation
└── README.md               # This file
```

## Code Style

This project follows the coding style of [bpmdetect](https://github.com/Tatsh/bpmdetect):

- C++23 standard
- No exceptions (use std::expected for error handling)
- Lowercase names without delimiters (e.g., `gamedatabase`, not `game_database`)
- clang-format for code formatting
- CMake for building
- vcpkg for dependencies

## Status

**Work in Progress**: This is a partial conversion focusing on the CLI framework and core utilities. Full functionality requires integrating or porting the external tools mentioned above.

### Implemented

- ✅ Project structure with CMake
- ✅ vcpkg dependency management
- ✅ Basic CLI interface
- ✅ String utilities
- ✅ Game database framework
- ✅ Type definitions

### TODO

- [ ] File format detection (ISO, WBFS, NKIT, NASOS, etc.)
- [ ] ISO reading and manipulation
- [ ] Image conversion (PNG/TGA)
- [ ] Audio conversion (WAV to BTSND)
- [ ] XML generation (app.xml, meta.xml)
- [ ] Encryption/decryption
- [ ] NFS conversion
- [ ] Package creation
- [ ] Nintendo CDN downloads
- [ ] Full integration of external tools

## Original Project

This is based on TeconMoon's WiiVC Injector, a fork of the popular Wii Virtual Console Injector for Wii U.

The original can be found here:
https://gbatemp.net/threads/release-wiivc-injector-script-gc-wii-homebrew-support.483577/

## License

[License to be determined based on original project]

## Contributing

Contributions are welcome! Please ensure:
- Code follows the .clang-format style
- No exceptions are used
- All functions use std::expected for error handling
- Platform-specific code is wrapped in appropriate #ifdef blocks
- Tests are added for new functionality
