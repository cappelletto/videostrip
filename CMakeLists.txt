cmake_minimum_required(VERSION 3.18)
project(videostrip VERSION 0.2.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==========================================
# Build output directories
# ==========================================
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ==========================================
# OpenCV Detection (Cross-OS)
# ==========================================
message(STATUS "Searching for OpenCV 4...")

# Try system-wide first
find_package(OpenCV 4 QUIET)

# Fallback to /usr/local or /opt/local
if(NOT OpenCV_FOUND)
    message(STATUS "OpenCV not found system-wide, trying /usr/local ...")
    find_package(OpenCV 4 QUIET
        PATHS /usr/local /opt/local
        NO_DEFAULT_PATH
    )
endif()

# If still not found, fail with user guidance
if(NOT OpenCV_FOUND)
    message(FATAL_ERROR
        "OpenCV 4.x was not found.\n"
        "On Debian/Ubuntu:\n"
        "  sudo apt update && sudo apt install libopencv-dev\n"
        "Or specify OpenCV_DIR:\n"
        "  cmake -DOpenCV_DIR=/path/to/opencv/build .."
    )
endif()

message(STATUS "OpenCV version: ${OpenCV_VERSION}")
message(STATUS "OpenCV include dirs: ${OpenCV_INCLUDE_DIRS}")

# ==========================================
# Add project modules (progressive build)
# ==========================================
add_subdirectory(videostrip_core)
# add_subdirectory(videostrip_utils)
add_subdirectory(videostrip_cli)
# add_subdirectory(videostrip_gui)
