cmake_minimum_required(VERSION 3.18)
project(videostrip VERSION 0.9.0 LANGUAGES CXX)

# ------------------------------------------------------------
# Licensing
# ------------------------------------------------------------
set(PROJECT_LICENSE "Apache-2.0 OR LGPL-3.0-or-later")
message(STATUS "License: ${PROJECT_LICENSE}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)

# ==========================================
# Build output directories
# ==========================================
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ---------- OpenCV bundling toggle ----------
option(VIDEOSTRIP_BUNDLE_OPENCV "Use vendored OpenCV under third_party/_install" ON)
set(VIDEOSTRIP_OPENCV_VERSION "4.12.0" CACHE STRING "Vendored OpenCV version")

# Absolute vendor paths (MUST match superbuild)
set(OPENCV_VENDOR_PREFIX   "${CMAKE_SOURCE_DIR}/third_party/_install/opencv-${VIDEOSTRIP_OPENCV_VERSION}")
set(OPENCV_VENDOR_CMAKEDIR "${OPENCV_VENDOR_PREFIX}/lib/cmake/opencv4")

# Prefer Config packages over Module/pkg-config
set(CMAKE_FIND_PACKAGE_PREFER_CONFIG ON)

message(STATUS "Searching for OpenCV ${VIDEOSTRIP_OPENCV_VERSION} (deterministic build via third_party/opencv)")

# If bundling, build vendor FIRST and then force OpenCV_DIR
if (VIDEOSTRIP_BUNDLE_OPENCV)
  # Make the prefix visible to the superbuild
  set(OPENCV_VENDOR_PREFIX "${OPENCV_VENDOR_PREFIX}" CACHE PATH "" FORCE)

  # Build the vendor only if it's not already installed
  if (NOT EXISTS "${OPENCV_VENDOR_CMAKEDIR}/OpenCVConfig.cmake")
    add_subdirectory(third_party/opencv ${CMAKE_BINARY_DIR}/_vendor/opencv)
    add_custom_target(_vendor_ocv_ready ALL
      DEPENDS opencv_vendor
      COMMENT "Vendored OpenCV built/installed at ${OPENCV_VENDOR_PREFIX}")
  else()
    message(STATUS "Vendored OpenCV already installed at ${OPENCV_VENDOR_PREFIX}")
  endif()

  # Clear any sticky cache to avoid finding /usr
  unset(OpenCV_DIR  CACHE)
  unset(OpenCV_ROOT CACHE)

  # Force CMake to use the vendor Config package
  set(OpenCV_DIR  "${OPENCV_VENDOR_CMAKEDIR}" CACHE PATH "OpenCV config dir" FORCE)
  set(OpenCV_ROOT "${OPENCV_VENDOR_PREFIX}"   CACHE PATH "OpenCV root"       FORCE)
endif()

# Locate OpenCV strictly in CONFIG mode (after forcing OpenCV_DIR if bundling)
set(VS_OPENCV_COMPONENTS core imgproc imgcodecs videoio features2d calib3d flann)
find_package(OpenCV 4 CONFIG REQUIRED COMPONENTS ${VS_OPENCV_COMPONENTS})

# Sanity check: ensure imported targets come from vendor when bundling
if (VIDEOSTRIP_BUNDLE_OPENCV AND EXISTS "${OPENCV_VENDOR_CMAKEDIR}/OpenCVConfig.cmake")
  get_target_property(_ocv_core_path opencv_core IMPORTED_LOCATION_RELEASE)
  if (NOT _ocv_core_path)
    get_target_property(_ocv_core_path opencv_core IMPORTED_LOCATION)
  endif()
  message(STATUS "opencv_core imported from: ${_ocv_core_path}")
  string(FIND "${_ocv_core_path}" "${OPENCV_VENDOR_PREFIX}" _hit)
  if (_hit EQUAL -1)
    message(FATAL_ERROR "Vendored OpenCV requested, but opencv_core resolves to: ${_ocv_core_path}\nVendor expected under: ${OPENCV_VENDOR_PREFIX}")
  endif()
endif()

message(STATUS "OpenCV version: ${OpenCV_VERSION}")
message(STATUS "OpenCV include dirs: ${OpenCV_INCLUDE_DIRS}")
message(STATUS "OpenCV libraries: ${OpenCV_LIBS}")

# ==========================================
# yaml-cpp detection. FetchContent fallback
# ==========================================
find_package(yaml-cpp CONFIG)

if (TARGET yaml-cpp)
  message(STATUS "yaml-cpp found (CONFIG package)")
else()
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

# At this point, a target named 'yaml-cpp' must exist
if(NOT TARGET yaml-cpp)
  message(FATAL_ERROR "yaml-cpp target not found even after FetchContent; aborting.")
endif()

# ==========================================
# Global include path (so <videostrip_core/...> works everywhere)
# ==========================================
include_directories(${CMAKE_SOURCE_DIR})

# ==========================================
# Add project modules
# ==========================================
add_subdirectory(videostrip_core)
if (TARGET opencv_vendor)
  add_dependencies(videostrip_core opencv_vendor)
endif()

add_subdirectory(videostrip_cli)
if (TARGET opencv_vendor)
  add_dependencies(videostrip_cli opencv_vendor)
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

# ========================================
# Install rules
# ========================================
install(TARGETS videostrip_cli RUNTIME DESTINATION bin)
install(TARGETS videostrip_core ARCHIVE DESTINATION lib LIBRARY DESTINATION lib)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/videostrip_core/
  DESTINATION include/videostrip_core
  FILES_MATCHING PATTERN "*.hpp")

install(DIRECTORY ${CMAKE_SOURCE_DIR}/meshroom/
        DESTINATION share/videostrip_plugin/meshroom
        FILES_MATCHING PATTERN "*.py" PATTERN "*.mg" PATTERN "config.json" PATTERN "__init__.py")

install(FILES configs/sample_config.yaml DESTINATION share/videostrip)
install(FILES ${CMAKE_SOURCE_DIR}/README.md ${CMAKE_SOURCE_DIR}/LICENSE DESTINATION share/doc/videostrip)

# RPATH so installed bins prefer bundled libs (when bundling)
if (VIDEOSTRIP_BUNDLE_OPENCV AND EXISTS "${OPENCV_VENDOR_CMAKEDIR}/OpenCVConfig.cmake")
  # Copy vendored libs next to our install if desired (optional – packaging may handle this)
  find_library(OPENCV_WORLD NAMES opencv_world PATHS "${OPENCV_VENDOR_PREFIX}/lib" NO_DEFAULT_PATH)
  if (OPENCV_WORLD)
    install(FILES "${OPENCV_WORLD}" DESTINATION lib)
  else()
    foreach(lib IN LISTS OpenCV_LIBS)
      get_target_property(_libloc ${lib} IMPORTED_LOCATION_RELEASE)
      if (NOT _libloc)
        get_target_property(_libloc ${lib} IMPORTED_LOCATION)
      endif()
      if (_libloc)
        install(FILES "${_libloc}" DESTINATION lib)
      endif()
    endforeach()
  endif()

  set(CMAKE_BUILD_WITH_INSTALL_RPATH OFF)
  set(CMAKE_INSTALL_RPATH_USE_LINK_PATH ON)
  set(CMAKE_INSTALL_RPATH "\$ORIGIN/../lib:/usr/lib/x86_64-linux-gnu:/usr/local/lib")
endif()

# -------------------------
# CPack configuration
# -------------------------
set(CPACK_PACKAGE_NAME "videostrip")
set(CPACK_PACKAGE_VENDOR "Rivermouth")
set(CPACK_PACKAGE_CONTACT "cappelletto@gmail.com")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Video transect utilities for 2D/3D mapping via structure-from-motion or mosaicking")
set(CPACK_PACKAGE_LICENSE "Apache-2.0 OR LGPL-3.0-or-later")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README  "${CMAKE_SOURCE_DIR}/README.md")
string(TOLOWER "${CMAKE_SYSTEM_NAME}" _sysname_lc)
# Nice archive base name: videostrip-X.Y.Z-Linux-x86_64, etc.
set(CPACK_PACKAGE_FILE_NAME "videostrip-${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_GENERATOR "TGZ;ZIP;DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Jose Cappelletto")
set(CPACK_DEBIAN_PACKAGE_SECTION "science")
set(CPACK_DEBIAN_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libstdc++6, libgcc-s1")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
include(CPack)
# End of CPack configuration