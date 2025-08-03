cmake_minimum_required(VERSION 3.18)
project(videostrip
  VERSION "0.2.1"
  DESCRIPTION "Smart video frame extraction for 2D/3D reconstruction pipelines"
  HOMEPAGE_URL "https://github.com/cappelletto/videostrip/"
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_GUI "Build the GUI frontend (requires OpenGL + GLFW + ImGui)" OFF)
option(BUILD_TESTS "Build unit tests" OFF)

# ==========================
# Build/packaging options
# ==========================
# Organize build outputs into bin/ and lib/
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)   # static libs
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)   # shared libs
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)   # executables


# ==========================
# Dependencies
# ==========================

# ========================================
# Git Info
# ========================================
execute_process(
    COMMAND git rev-parse HEAD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE PROJECT_COMMIT_ID
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
    COMMAND git rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE PROJECT_COMMIT_ID_SHORT
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# ==========================
# OpenCV Detection (Cross-OS)
# ==========================
message(STATUS "Searching for OpenCV 4...")

# Try system-wide first
find_package(OpenCV 4 QUIET)

# Fallback to /usr/local if not found
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
        "On Debian/Ubuntu, install with:\n"
        "  sudo apt update && sudo apt install libopencv-dev\n"
        "Or specify OpenCV_DIR to your custom build, e.g.:\n"
        "  cmake -DOpenCV_DIR=/path/to/opencv/build .."
    )
endif()

message(STATUS "OpenCV version: ${OpenCV_VERSION}")

# ==========================
# Core Library
# ==========================
add_library(videostrip_core
    libvideostrip_core/vscore.cpp
)
target_include_directories(videostrip_core
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libvideostrip_core
)
target_link_libraries(videostrip_core PRIVATE ${OpenCV_LIBS})
target_compile_definitions(videostrip_core PUBLIC GIT_COMMIT="${GIT_INFO}")

# ==========================
# Utils Library
# ==========================
add_library(videostrip_utils
    libvideostrip_utils/helper.cpp
)
target_include_directories(videostrip_utils
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libvideostrip_utils
)
target_link_libraries(videostrip_utils PRIVATE videostrip_core ${OpenCV_LIBS})

# ==========================
# CLI Frontend
# ==========================
add_executable(videostrip-cli src/videostrip-cli.cpp)
target_link_libraries(videostrip-cli PRIVATE videostrip_core videostrip_utils ${OpenCV_LIBS})

# ==========================
# Optional GUI Frontend
# ==========================
if(BUILD_GUI)
    find_package(OpenGL REQUIRED)
    find_package(PkgConfig REQUIRED)
    pkg_search_module(GLFW REQUIRED glfw3)

    set(IMGUIFILEDIALOG_DIR external/ImGuiFileDialog)
    set(IMGUI_DIR external/imgui)
    set(IMGUI_SOURCES
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_demo.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
        ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
    )

    add_executable(videostrip-gui
        src/videostrip-gui.cpp
        src/gui/vsgui.cpp
        external/ImGuiFileDialog/ImGuiFileDialog.cpp
        ${IMGUI_SOURCES}
    )

    target_include_directories(videostrip-gui PRIVATE
        ${OpenGL_INCLUDE_DIR}
        ${GLFW_INCLUDE_DIRS}
        ${IMGUIFILEDIALOG_DIR}
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
    )
    target_link_libraries(videostrip-gui PRIVATE
        videostrip_core
        videostrip_utils
        ${OPENGL_LIBRARIES}
        ${GLFW_LIBRARIES}
        ${CMAKE_DL_LIBS}
    )
endif()

# ========================================
# Display project status
# ========================================
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
message(STATUS "----------------------------------------")
message(STATUS "")
message(STATUS "--- CMake Options ---")
message(STATUS "BUILD_GUI                      : ${BUILD_GUI}")
message(STATUS "BUILD_TESTS                    : ${BUILD_TESTS}")
message(STATUS "----------------------------------------")
message(STATUS "")
message(STATUS "--- Dependencies ---")
message(STATUS "OpenCV version                 : ${OpenCV_VERSION}")
message(STATUS "OpenCV include dirs            : ${OpenCV_INCLUDE_DIRS}")
message(STATUS "OpenCV libraries               : ${OpenCV_LIBS}")
if(BUILD_GUI)
    message(STATUS "OpenGL libraries               : ${OPENGL_LIBRARIES}")
    message(STATUS "GLFW libraries                 : ${GLFW_LIBRARIES}")
endif()
message(STATUS "----------------------------------------")

# ==========================
# Tests
# ==========================
if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
