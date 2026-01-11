# C++23 Conversion Implementation Notes

## Overview

This document outlines the C++23 conversion of the TeconMoon's WiiVC Injector from C# Windows Forms to a portable command-line application.

## Design Principles

Following the bpmdetect reference project:

1. **No Exceptions**: All error handling uses `std::expected<T, ErrorCode>`
2. **C++23 Standard**: Using modern C++ features while maintaining portability
3. **Lowercase Naming**: All identifiers use lowercase without delimiters (e.g., `gamedatabase`, not `game_database`)
4. **Cross-Platform**: Code is designed to work on Windows, Linux, and macOS
5. **vcpkg Dependencies**: All third-party libraries managed through vcpkg
6. **CMake Build System**: Industry-standard build configuration

## Implemented Components

### Core Utilities

1. **stringutils.h** - String manipulation utilities
   - `removeDiacritics()` - Remove accented characters
   - `removeSpecialChars()` - Filter to ASCII
   - `replaceAt()` - Character replacement
   - `hexToBytes()` / `bytesToHex()` - Hex conversion
   - `sanitizeFilename()` - Remove invalid filename characters

2. **gamedatabase.h/cpp** - Game database lookup
   - `getName()` - Get game name from ID
   - `getIds()` - Get IDs for a game name
   - `getAlternativeIds()` - Find regional variants
   - Embedded database (minimal sample, would load from resource)

3. **fileformat.h/cpp** - File format detection
   - Detects: ISO, WBFS, NKIT, NASOS, DOL, GCM
   - `readGameId()` - Extract game ID from disc image
   - `readGameName()` - Extract internal game name
   - `readGameType()` - Determine Wii vs GameCube
   - Handles format-specific offsets (WBFS at 0x200, NASOS variants)

4. **crypto.h/cpp** - Cryptography and key verification
   - MD5 hash computation (files and data)
   - Key verification for Wii U Common Key, Title Key, Ancast Key
   - Hash comparison with normalization

### CLI Application

**main.cpp** - Command-line interface
- Argument parsing with CLI11
- File type detection and game info extraction
- Key verification
- Verbose output option
- Structured error handling

## Dependencies

Via vcpkg:
- **CLI11** - Command-line parsing
- **fmt** - String formatting
- **pugixml** - XML parsing (for future meta.xml generation)
- **curl** - HTTP downloads (for future JNUSTool integration)
- **openssl** - Cryptography (MD5, encryption)
- **zlib** - Compression

## Platform-Specific Code

Currently minimal platform-specific code. Future additions will use:

```cpp
#ifdef _WIN32
    // Windows-specific code
#elif defined(__APPLE__)
    // macOS-specific code
#else
    // Linux/Unix-specific code
#endif
```

## External Tools Integration

The original C# application relied on these external tools:

1. **wit** (Wiimms ISO Tools) - ISO manipulation
   - Status: Needs integration or library port
   - Alternative: Direct ISO parsing

2. **nfs2iso2nfs** - NFS format conversion
   - Status: Needs integration or library port
   
3. **JNUSTool** - Nintendo CDN downloads
   - Status: Could implement HTTP download logic
   
4. **NUSPacker** - WUP package creation
   - Status: Needs porting or integration

5. **Image conversion** - PNG/TGA conversion
   - Can use: stb_image, libpng, libtga

6. **Audio conversion** - WAV to BTSND
   - Needs research on BTSND format

## Error Handling Pattern

All functions that can fail return `Result<T>`:

```cpp
Result<std::string> readGameName(const std::filesystem::path &path) {
    std::ifstream file(path);
    if (!file) {
        return std::unexpected(ErrorCode::FileNotFound);
    }
    // ... process file ...
    return gameName;
}

// Usage:
auto result = readGameName(path);
if (!result) {
    fmt::print("Error: {}\n", errorToString(result.error()));
    return 1;
}
auto name = *result;
```

## Remaining Work

### High Priority
- [ ] ISO reading/writing library or wit integration
- [ ] Image format conversion (PNG → TGA)
- [ ] XML generation (app.xml, meta.xml)
- [ ] NFS conversion logic
- [ ] Encryption/decryption implementation

### Medium Priority
- [ ] Audio conversion (WAV → BTSND)
- [ ] Nintendo CDN download integration
- [ ] Package creation (WUP format)
- [ ] Progress reporting for long operations

### Low Priority
- [ ] Configuration file support
- [ ] Unit tests
- [ ] Integration tests
- [ ] Complete game database

## Code Style

Enforced by `.clang-format`:
- 100 column limit
- 4-space indentation
- No tabs
- Attach braces
- Pointer alignment right
- All lowercase identifiers without delimiters

## Building

```bash
# Configure
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake

# Build
cmake --build build

# Install
cmake --install build --prefix /usr/local
```

## Testing

When tests are added:

```bash
cmake -B build -S . -DBUILD_TESTS=ON
cmake --build build
ctest --test-dir build
```

## Future Enhancements

1. **Library Mode**: Extract core logic into a library with C API for binding to other languages
2. **GUI Option**: Optional Qt/GTK frontend (maintaining CLI as default)
3. **Batch Processing**: Process multiple games in one invocation
4. **Parallel Processing**: Multi-threaded conversion for multiple games
5. **Resume Support**: Save state for interrupted conversions

## Migration from C#

| C# Component | C++23 Equivalent |
|--------------|------------------|
| WinForms GUI | CLI11 command-line |
| .NET Framework | C++ Standard Library |
| C# exceptions | std::expected |
| Registry storage | Config file |
| Process.Start() | std::system() or library integration |
| Image.FromFile() | stb_image or similar |
| ZipFile class | zlib or libzip |
| MD5CryptoServiceProvider | OpenSSL MD5 |

## License

To be determined based on original project licensing and third-party dependencies.
