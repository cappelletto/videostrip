cmake_minimum_required(VERSION 3.18)
project(videostrip
  VERSION "0.2.0"
  DESCRIPTION "Smart video frame extraction for 2D/3D reconstruction pipelines"
  HOMEPAGE_URL "https://github.com/cappelletto/videostrip/"
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_GUI "Build the GUI frontend (requires OpenGL + GLFW + ImGui)" OFF)
option(BUILD_TESTS "Build unit tests" OFF)

# ==========================
# Dependencies
# ==========================
find_package(OpenCV 4 REQUIRED
    PATHS /usr/local
    NO_DEFAULT_PATH
)
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

# ==========================
# Git Info
# ==========================
execute_process(
    COMMAND git describe --abbrev=4 --dirty --always --tags
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_INFO
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# ==========================
# Tests
# ==========================
if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
