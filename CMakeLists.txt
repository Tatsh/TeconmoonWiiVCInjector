cmake_minimum_required(VERSION 3.24 FATAL_ERROR)
project(wiivc-injector VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Build executable
add_executable(wiivc-injector
    src/main.c
    src/iso_reader.c
    src/metadata.c
    src/utils.c
)

target_include_directories(wiivc-injector PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(wiivc-injector PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    $<$<C_COMPILER_ID:MSVC>:/W4>
)

# Platform-specific settings
if(WIN32)
    target_compile_definitions(wiivc-injector PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

# Installation
install(TARGETS wiivc-injector DESTINATION bin)
