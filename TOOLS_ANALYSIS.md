# External Tools Analysis

This document analyzes the external tools used in the original C# application and their potential for integration or porting to C.

## Tools Used

### 1. Wiimms ISO Tools (wit)
**Repository**: https://github.com/Wiimm/wiimms-iso-tools
**Language**: C
**License**: GPL-2.0
**Purpose**: Extract, rebuild, and modify Wii/GameCube ISO files
**Integration Feasibility**: ★★★★☆ (High)
- **Pros**: Written in C, mature codebase, well-maintained
- **Cons**: Large codebase (~100k+ lines), complex functionality
- **Recommendation**: Can be integrated as a library or linked dependency. The core ISO manipulation code could potentially be extracted.

### 2. sign_c2w_patcher
**Repository**: https://github.com/FIX94/sign_c2w_patcher
**Language**: C (Wii U homebrew)
**License**: Not specified
**Purpose**: Apply signature patches and redirect cafe2wii
**Integration Feasibility**: ★★☆☆☆ (Low)
- **Pros**: Written in C
- **Cons**: Wii U specific (PowerPC), requires devkitPPC toolchain
- **Recommendation**: Not directly portable. The desktop version (c2w_patcher.exe) is likely a separate binary. Need to find the desktop patcher source.

### 3. nfs2iso2nfs
**Repository**: Multiple forks exist, need to identify canonical source
**Language**: Likely C/C++
**Purpose**: Convert between NFS (Wii U format) and ISO formats
**Integration Feasibility**: ★★★☆☆ (Medium)
- **Status**: Repository access issues, need to investigate further
- **Recommendation**: Critical tool for the conversion process, must be integrated or called

### 4. JNUSTool
**Type**: Java application
**Purpose**: Download base files from Nintendo CDN
**Integration Feasibility**: ★☆☆☆☆ (Very Low)
- **Cons**: Java-based, requires JRE, network functionality
- **Recommendation**: Keep as external dependency or reimplement download logic in C using libcurl

### 5. NUSPacker
**Type**: Java application
**Purpose**: Encrypt and pack content into WUP format
**Integration Feasibility**: ★☆☆☆☆ (Very Low)
- **Cons**: Java-based, complex encryption logic
- **Recommendation**: Keep as external dependency or find/create C alternative

### 6. Image Conversion Tools (png2tgacmd, tga2pngcmd)
**Purpose**: Convert between PNG and TGA formats
**Integration Feasibility**: ★★★★★ (Very High)
- **Recommendation**: Use existing C libraries:
  - stb_image.h (single-header library for image loading)
  - stb_image_write.h (single-header library for image writing)
  - Both support PNG and TGA formats
  - No external dependencies needed

### 7. SoX (Sound eXchange)
**Language**: C
**Purpose**: Audio format conversion
**Integration Feasibility**: ★★★☆☆ (Medium)
- **Recommendation**: Use libsndfile or libsox as a library dependency

### 8. wbfs_file
**Purpose**: WBFS format handling
**Integration Feasibility**: ★★★★☆ (High)
- **Recommendation**: WBFS format is well-documented, can be implemented in C or use existing libwbfs

### 9. ConvertToISO (NKIT)
**Purpose**: Convert NKIT format to ISO
**Integration Feasibility**: ★★☆☆☆ (Low)
- **Recommendation**: Proprietary NKIT format, may need to keep as external tool

## Overall Strategy

### Phase 1: Minimal Implementation (Current)
- Create CLI interface in C17
- Call external tools as subprocesses
- Handle metadata generation and file coordination in C

### Phase 2: Partial Integration
- Integrate stb_image for image conversion (replaces png2tgacmd/tga2pngcmd)
- Implement WBFS reading natively using WBFS format specification
- Use libsndfile for audio conversion (replaces sox)

### Phase 3: Advanced Integration (Future)
- Extract and integrate relevant parts of Wiimms ISO Tools
- Implement NFS format handling in C
- Create native encryption/packing (reverse engineer or find open source alternative to NUSPacker)

## Dependencies Needed via vcpkg

For Phase 2 implementation:
- None required for stb (header-only)
- libsndfile (for audio conversion)
- libcurl (for downloading base files, replacing JNUSTool)
- openssl (for encryption operations)
- zlib (for compression)

## Conclusion

A fully native C17 implementation is feasible but requires significant effort. The recommended approach is:

1. **Short term**: CLI wrapper calling external tools (current implementation)
2. **Medium term**: Replace image/audio conversion with C libraries
3. **Long term**: Implement ISO/NFS handling natively, find/create open encryption tools

The most critical missing piece is a portable NFS format handler and WUP packer in C.
