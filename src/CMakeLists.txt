add_executable(wiivc-injector
    main.cpp
    utils/gamedatabase.cpp
    utils/fileformat.cpp
    utils/crypto.cpp
    utils/imageconvert.cpp
    utils/xmlgen.cpp
    utils/download.cpp
)

target_include_directories(wiivc-injector PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${STB_INCLUDE_DIRS}
)

target_link_libraries(wiivc-injector PRIVATE
    CLI11::CLI11
    fmt::fmt
    pugixml::pugixml
    CURL::libcurl
    OpenSSL::SSL
    OpenSSL::Crypto
)

# Set C++23 standard
set_target_properties(wiivc-injector PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Install target
install(TARGETS wiivc-injector
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
