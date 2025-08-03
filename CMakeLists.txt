# videostrip modular build
cmake_minimum_required(VERSION 3.18)

project(videostrip)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Core library (for frame extraction, metadata, etc.)
add_library(videostrip_core
    libvideostrip_core/vscore.cpp
    # Add more core sources here
)
target_include_directories(videostrip_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/libvideostrip_core
    ${CMAKE_CURRENT_SOURCE_DIR}/libvideostrip_utils
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Utils library (for helpers, logging, etc.)
add_library(videostrip_utils
    libvideostrip_utils/helper.cpp
    # Add more utils sources here
)
target_include_directories(videostrip_utils PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/libvideostrip_utils
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# CLI frontend (thin wrapper)
add_executable(videostrip-cli src/videostrip-cli.cpp)
target_link_libraries(videostrip-cli PRIVATE videostrip_core videostrip_utils)

# GUI frontend (thin wrapper)
add_executable(videostrip-gui src/videostrip-gui.cpp)
target_link_libraries(videostrip-gui PRIVATE videostrip_core videostrip_utils)

# TODO: Add OpenCV, ImGui, and other dependencies as needed
# find_package(OpenCV REQUIRED)
# target_link_libraries(videostrip_core PRIVATE ${OpenCV_LIBS})
# ...

# Prepare for tests
enable_testing()
add_subdirectory(tests)
#     message(STATUS "    include path: ${CUDA_INCLUDE_DIRS}")
#   endif(CUDA_FOUND)
# endif()

# Find OpenCV, you may need to set OpenCV_DIR variable
# to the absolute path to the directory containing OpenCVConfig.cmake file
find_package(OpenCV 4   REQUIRED 
                        PATHS /usr/local /local/software/opencv/4.5.3
                        NO_DEFAULT_PATH)

# If the package has been found, several variables will be set, you can find the full list with descriptions
# in the OpenCVConfig.cmake file.
message("${BoldGreen}" "OpenCV library status:" "${ColourReset}")
message(STATUS "    version: ${OpenCV_VERSION}")
message(STATUS "    libraries: ${OpenCV_LIBS}")
message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")

# OpenGL is used as a rendering backend, so we need to find it
find_package(OpenGL REQUIRED)
if (OPENGL_FOUND)
  message("${BoldGreen}" "OpenGL library status:" "${ColourReset}")
  message(STATUS "    version: ${OPENGL_VERSION}")
  message(STATUS "    libraries: ${OPENGL_LIBRARIES}")
  message(STATUS "    include path: ${OPENGL_INCLUDE_DIR}")
endif()

# find_package(glfw3 3.3 REQUIRED)  # current release for cmake is pretty incomplete. Use pkg-config instead
# message(STATUS "Found GLFW3 in ${GLFW3_INCLUDE_DIR}")
# message(STATUS "Found GLFW in ${GLFW_LIBRARIES}")

find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)

include_directories(${GLFW_INCLUDE_DIRS})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${GLFW_INCLUDE_DIRS}")

set (IMGUIFILEDIALOG_DIR external/ImGuiFileDialog)

set (IMGUI_DIR      external/imgui)
set (IMGUI_SOURCES  ${IMGUI_DIR}/imgui.cpp
                    ${IMGUI_DIR}/imgui_draw.cpp
                    ${IMGUI_DIR}/imgui_demo.cpp
                    ${IMGUI_DIR}/imgui_widgets.cpp
                    ${IMGUI_DIR}/imgui_tables.cpp
                    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
                    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp)

# Greedy include of all headers
file(GLOB PROJECT_HEADERS include/*.h include/*.hpp external/*.h external/*.hpp external/*.hxx)

include_directories(BEFORE ../include
                    include
                    ${OpenCV_INCLUDE_DIRS}
                    ${IMGUIFILEDIALOG_DIR}
                    ${IMGUI_DIR}
                    ${IMGUI_DIR}/backends
                    )

# Retrieve git commit information, forward it as compiler flags so they can be used during building time
exec_program(
    "git"
    ${CMAKE_CURRENT_SOURCE_DIR}
    ARGS "describe --abbrev=4 --dirty --always --tags"
    OUTPUT_VARIABLE GIT_INFO )
add_definitions( -DGIT_COMMIT="${GIT_INFO}" ) # Forward current git info as a single string
add_definitions( -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}" ) # Forward current git info as a single string

# collect all the libraries in a single variable
set(CORE_LIBS ${OpenCV_LIBS} )
set(GUI_LIBS  ${OPENGL_LIBRARIES} ${GLFW_LIBRARIES} ${IMGUI_LIBS})

message ("${Cyan} Building system")
message ("${BoldBlue} Building core libraries: ${ColourReset}")
message ("CORE_LIBS: ${CORE_LIBS}")
message ("${BoldBlue} Building GUI libraries: ${ColourReset}")
message ("GUI_LIBS: ${GUI_LIBS}")

############################ videostrip ####################
add_executable(${PROJECT_NAME}  src/videostrip-cli.cpp
                                src/helper.cpp
                                ${PROJECT_HEADERS})

############################ videostrip ####################
add_executable(videostrip-gui 
                            src/videostrip-gui.cpp
                            src/helper.cpp
                            src/gui/vsgui.cpp
                            src/videostrip/vscore.cpp
                            external/ImGuiFileDialog/ImGuiFileDialog.cpp
                            ${IMGUI_SOURCES})
                            #${PROJECT_HEADERS}
                            # )

target_compile_options(${PROJECT_NAME}  PUBLIC -std=c++17 -mavx2) # the global flags may already have been set with std=c++17 
target_link_libraries (${PROJECT_NAME}  ${CORE_LIBS} )

target_compile_options(videostrip-gui   PUBLIC -std=c++17 -mavx2) # the global flags may already have been set with std=c++17 
target_link_libraries (videostrip-gui   ${CORE_LIBS} ${GUI_LIBS} ${CMAKE_DL_LIBS} )
# target_link_libraries (videostrip-gui   ${CORE_LIBS} ${GUI_LIBS} ${CMAKE_DL_LIBS} glfw glut glu glfw3)

install (TARGETS ${PROJECT_NAME}        DESTINATION $ENV{HOME}/bin)
install (TARGETS videostrip-gui         DESTINATION $ENV{HOME}/bin)
# configure_file(scripts/tiff.convert2png.sh  $ENV{HOME}/bin/scripts)
