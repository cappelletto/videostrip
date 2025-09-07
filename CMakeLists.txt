cmake_minimum_required(VERSION 3.18)
project(videostrip VERSION 0.7.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==========================================
# Build output directories
# ==========================================
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ==========================================
# Build options
# ==========================================
option(BUILD_TESTS "Build unit/integration tests" ON)
option(BUILD_GUI "Build GUI module" OFF)


message(STATUS "--------------------")
message(STATUS "Configuring project: ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "--------------------")

# ==========================================
# OpenCV Detection (Cross-OS)
# ==========================================
message(STATUS "Searching for OpenCV 4.5 or higher...")

# Try system-wide first
find_package(OpenCV 4 QUIET)

# Fallback to /usr/local or /opt/local
if(NOT OpenCV_FOUND)
    message(STATUS "OpenCV not found system-wide, trying /usr/local ...")
    find_package(OpenCV 4.5 QUIET
        PATHS /usr/local /opt/local
        NO_DEFAULT_PATH
    )
endif()

# If still not found, fail with user guidance
if(NOT OpenCV_FOUND)
    message(FATAL_ERROR
        "OpenCV 4.5 or higher was not found.\n"
        "On Debian/Ubuntu:\n"
        "  sudo apt update && sudo apt install libopencv-dev\n"
        "Or specify OpenCV_DIR:\n"
        "  cmake -DOpenCV_DIR=/path/to/opencv/build .."
    )
endif()

message(STATUS "OpenCV version: ${OpenCV_VERSION}")
message(STATUS "OpenCV include dirs: ${OpenCV_INCLUDE_DIRS}")

# ==========================================
# Global include path (so <videostrip_core/...> works everywhere)
# ==========================================
# All headers live in subfolders inside ${CMAKE_SOURCE_DIR}
include_directories(${CMAKE_SOURCE_DIR})

# ==========================================
# Promoted to top-level as it is needed by multiple modules
find_package(yaml-cpp QUIET)
if(NOT yaml-cpp_FOUND)
  message(STATUS "yaml-cpp not found; fetching...")
  include(FetchContent)
  FetchContent_Declare(yaml_cpp GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git GIT_TAG 0.8.0)
  FetchContent_MakeAvailable(yaml_cpp)
endif()

# ==========================================
# Add project modules (progressive build)
# ==========================================
add_subdirectory(videostrip_core)
# add_subdirectory(videostrip_utils)
add_subdirectory(videostrip_cli)

if(BUILD_GUI)
    add_subdirectory(videostrip_gui)
endif()

if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

# ========================================
# Display project status (if options selected)
# ========================================

option(PROJECT_SUMMARY "Show project configuration summary" OFF)

if (PROJECT_SUMMARY)
  message(STATUS "----------------------------------------")
  message(STATUS "Project Configuration Summary")
  message(STATUS "----------------------------------------")
  message(STATUS "")
  message(STATUS "--- Project Information ---")
  message(STATUS "Name                           : ${PROJECT_NAME}")
  message(STATUS "Version                        : ${PROJECT_VERSION}")
  message(STATUS "Commit id                      : ${PROJECT_COMMIT_ID}")
  message(STATUS "Commit id short                : ${PROJECT_COMMIT_ID_SHORT}")
  message(STATUS "Source Dir                     : ${CMAKE_SOURCE_DIR}")
  message(STATUS "Build Dir                      : ${CMAKE_BINARY_DIR}")
  message(STATUS "----------------------------------------")
  message(STATUS "")
  message(STATUS "--- System Information ---")
  message(STATUS "Processor                      : ${CMAKE_SYSTEM_PROCESSOR}")
  message(STATUS "System Name                    : ${CMAKE_SYSTEM_NAME}")
  message(STATUS "Version                        : ${CMAKE_SYSTEM_VERSION}")
  message(STATUS "Compiler                       : ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
  message(STATUS "----------------------------------------")
  message(STATUS "")
  message(STATUS "--- Build Settings ---")
  if(CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Build type                     : Multi-configuration (${CMAKE_CONFIGURATION_TYPES})")
  else()
    message(STATUS "Build type                     : ${CMAKE_BUILD_TYPE}")
  endif()
  message(STATUS "C++ Standard                   : ${CMAKE_CXX_STANDARD}")
  message(STATUS "CMake Module Path              : ${CMAKE_MODULE_PATH}")
endif()
message(STATUS "----------------------------------------")
