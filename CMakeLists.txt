cmake_minimum_required(VERSION 3.20)
project(EdgeGlow VERSION 1.0.0 LANGUAGES CXX)

# C++17 Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# MSVC-specific flags
if(MSVC)
    add_compile_options(/W4 /WX-) # Warning level 4, don't treat warnings as errors
    add_compile_options(/MP) # Multi-processor compilation
    add_definitions(-DUNICODE -D_UNICODE) # Unicode support
endif()

# Source files for Phase 2
set(SOURCES
    src/main.cpp
    src/render/OverlayWindow.cpp
    src/render/GlowRenderer.cpp
    src/audio/AudioCapture.cpp
)

set(HEADERS
    src/render/OverlayWindow.h
    src/render/GlowRenderer.h
    src/audio/AudioCapture.h
    src/utils/CircularBuffer.h
)

# Create executable (remove WIN32 to show console for debugging)
add_executable(EdgeGlow ${SOURCES} ${HEADERS})

# Windows-specific libraries
target_link_libraries(EdgeGlow PRIVATE
    d2d1 # Direct2D
    dwrite # DirectWrite
    dcomp # DirectComposition
    windowscodecs # WIC (Windows Imaging Component)
)

# Set output directory
set_target_properties(EdgeGlow PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Copy resources to output directory (for later phases)
# file(COPY resources DESTINATION "${CMAKE_BINARY_DIR}/bin")
message(STATUS "EdgeGlow Phase 2 - Audio Capture")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")