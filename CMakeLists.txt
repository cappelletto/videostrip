cmake_minimum_required(VERSION 3.18)
project(videostrip VERSION 0.8.0 LANGUAGES CXX)

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
# option(BUILD_GUI "Build GUI module" OFF)
option(BUILD_MANPAGE "Generate manpage for CLI tool (requires help2man)" OFF)


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
# yaml-cpp detection. FetchContent fallback
# ==========================================
find_package(yaml-cpp CONFIG)

if (TARGET yaml-cpp)
    message(STATUS "yaml-cpp found (CONFIG package)")
    message(STATUS "yaml-cpp include dirs: ${yaml-cpp_INCLUDE_DIRS}")
else()
    message(STATUS "yaml-cpp not found (CONFIG package)")
  message(STATUS ">>>> yaml-cpp not found; fetching source with FetchContent...")
  include(FetchContent)

  # Keep the fetched build small & static
  set(YAML_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
  set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
  set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
  set(YAML_CPP_INSTALL     OFF CACHE BOOL "" FORCE)
  # MSVC runtime per your project policy
  set(YAML_MSVC_SHARED_RT  ON  CACHE BOOL "" FORCE)

  FetchContent_Declare(yaml_cpp
    GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
    GIT_TAG        0.8.0
  )
  FetchContent_MakeAvailable(yaml_cpp)
endif()

# At this point, a target named 'yaml-cpp' must exist (from either path).
# If not, abort with error.
if(NOT TARGET yaml-cpp)
  message(FATAL_ERROR "yaml-cpp target not found even after FetchContent; aborting.")
endif()

# ==========================================
# Global include path (so <videostrip_core/...> works everywhere)
# ==========================================
# All headers live in subfolders inside ${CMAKE_SOURCE_DIR}
include_directories(${CMAKE_SOURCE_DIR})

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

# -------------------------
# Install rules (what goes into the package)
# -------------------------
# Binaries
install(TARGETS videostrip_cli
  RUNTIME DESTINATION bin
)

# Libraries (adjust if SHARED; this is STATIC right now)
install(TARGETS videostrip_core
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
)

# Public headers (install only .hpp files from videostrip_core/)
# If the headers live in subdirs (feature/, logging/), this will include them.
install(DIRECTORY ${CMAKE_SOURCE_DIR}/videostrip_core/
  DESTINATION include/videostrip_core
  FILES_MATCHING PATTERN "*.hpp"
)

# Install shared assets
install(FILES configs/sample_config.yaml
        DESTINATION share/videostrip)
# Licenses / docs
install(FILES ${CMAKE_SOURCE_DIR}/README.md ${CMAKE_SOURCE_DIR}/LICENSE
  DESTINATION share/doc/videostrip
)
# -------------------------
# Manpage generation (optional)
# -------------------------
if (BUILD_MANPAGE)
    message(STATUS "Manpage generation enabled")
    find_program(HELP2MAN_EXECUTABLE help2man)

    if(HELP2MAN_EXECUTABLE)
        # Create packaging dir if it does not exist
        file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/packaging)
        add_custom_command(
            OUTPUT ${CMAKE_BINARY_DIR}/packaging/videostrip.1
            COMMAND ${HELP2MAN_EXECUTABLE} --no-discard-stderr --output=${CMAKE_BINARY_DIR}/packaging/videostrip.1 $<TARGET_FILE:videostrip_cli>
            DEPENDS videostrip_cli
            COMMENT "Generating manpage for videostrip_cli"
            VERBATIM
        )

        add_custom_target(manpage ALL
            DEPENDS ${CMAKE_BINARY_DIR}/packaging/videostrip.1
        )

        install(FILES ${CMAKE_BINARY_DIR}/packaging/videostrip.1 DESTINATION share/man/man1)
        else()
        message(WARNING "help2man not found: manpage will not be generated automatically.")
    endif()

else()
  message(STATUS "Manpage generation disabled")
endif()


# -------------------------
# CPack configuration
# -------------------------

set(CPACK_PACKAGE_NAME "videostrip")
set(CPACK_PACKAGE_VENDOR "Rivermouth")
set(CPACK_PACKAGE_CONTACT "cappelletto@gmail.com")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Video transect utilities for 2D/3D mapping (CLI)")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README  "${CMAKE_SOURCE_DIR}/README.md")
# Nice archive base name: videostrip-0.7.0-Linux-x86_64, etc.
string(TOLOWER "${CMAKE_SYSTEM_NAME}" _sysname_lc)
set(CPACK_PACKAGE_FILE_NAME
  "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}"
)

# Source + binary archives
set(CPACK_GENERATOR "TGZ;ZIP;DEB")

# Debian metadata (tune as we expand shared libs)
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Jose Cappelletto")           # required
set(CPACK_DEBIAN_PACKAGE_SECTION "science")
set(CPACK_DEBIAN_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})

# Runtime deps if linking dynamically; keep minimal first, then refine with ldd
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libstdc++6, libgcc-s1")
# Automate depedencies based on linked shared libs
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
# Avoid the classic error: ensure CPack knows the project
set(CPACK_PACKAGE_FILE_NAME "videostrip-${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")

include(CPack)
# End of CPack configuration
