cmake_minimum_required(VERSION 3.18)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ========================================
# CMake Project
# ========================================
# Set project meta data
project(videostrip
  VERSION "0.2.0"
  DESCRIPTION "Smart video frame extraction for 2D/3D reconstruction pipelines"
  HOMEPAGE_URL "https://github.com/cappelletto/videostrip/"
  LANGUAGES CXX
)

# ========================================
# Project Options
# ========================================
option(BUILD_GUI "Build the GUI frontend (requires OpenGL + GLFW + ImGui)" OFF)
option(BUILD_TESTS "Build unit testing suite (unavailable)")

# ========================================
# Dependencies
# ========================================
# -------------------------
# OpenCV (always required)
# -------------------------
find_package(OpenCV 4 REQUIRED
    PATHS /usr/local
    NO_DEFAULT_PATH
)
message(STATUS "OpenCV version: ${OpenCV_VERSION}")
message(STATUS "OpenCV libraries: ${OpenCV_LIBS}")
message(STATUS "OpenCV include path: ${OpenCV_INCLUDE_DIRS}")


# -------------------------
# Core library
# -------------------------
add_library(videostrip_core
    libvideostrip_core/vscore.cpp
    # Add more core sources here
)
target_include_directories(videostrip_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/libvideostrip_core
    ${CMAKE_CURRENT_SOURCE_DIR}/libvideostrip_utils
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# -------------------------
# Utils library
# -------------------------
add_library(videostrip_utils
    libvideostrip_utils/helper.cpp
    # Add more utils sources here
)
target_include_directories(videostrip_utils PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/libvideostrip_utils
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
)

# -------------------------
# CLI frontend
# -------------------------
add_executable(videostrip-cli src/videostrip-cli.cpp)
target_link_libraries(videostrip-cli PRIVATE videostrip_core videostrip_utils)


target_link_libraries(videostrip_core PRIVATE ${OpenCV_LIBS})
target_include_directories(videostrip_core PUBLIC ${OpenCV_INCLUDE_DIRS})

# -------------------------
# Optional GUI frontend
# -------------------------
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

    target_include_directories(videostrip-gui PUBLIC
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

# -------------------------
# Embed Git info
# -------------------------
execute_process(
    COMMAND git describe --abbrev=4 --dirty --always --tags
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_INFO
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
add_definitions(-DGIT_COMMIT="${GIT_INFO}")
add_definitions(-DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}")

# -------------------------
# Optional Tests
# -------------------------
if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
