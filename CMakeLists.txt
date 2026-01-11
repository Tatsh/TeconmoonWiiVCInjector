cmake_minimum_required(VERSION 3.31)
project(
    wiivc-injector
    LANGUAGES CXX
    VERSION 1.0.0
    DESCRIPTION "Wii Virtual Console Injector for Wii U"
    HOMEPAGE_URL "https://github.com/Tatsh/TeconmoonWiiVCInjector")

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_TESTS "Build tests." OFF)

# Find dependencies
find_package(CLI11 CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(pugixml CONFIG REQUIRED)
find_package(CURL REQUIRED)
find_package(OpenSSL REQUIRED)
find_path(STB_INCLUDE_DIRS "stb_image.h")

# Platform-specific libraries
if(WIN32)
    # Windows-specific libraries if needed
elseif(APPLE)
    # macOS-specific libraries if needed
else()
    # Linux-specific libraries if needed
endif()

add_subdirectory(src)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
