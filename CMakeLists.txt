cmake_minimum_required(VERSION 3.18)  # coud be lower, still to be decided

if(NOT DEFINED CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE Release CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebug RelWithDebInfo MinSizeRel."
      FORCE)
endif()

set(CMAKE_COLOR_MAKEFILE ON)  # For fancy colouring scheme
if (${FORCE_COLORED_OUTPUT})
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        target_compile_options (project_options INTERFACE -fdiagnostics-color=always)
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        target_compile_options (project_options INTERFACE -fcolor-diagnostics)
    endif ()
endif ()

if(NOT WIN32)
  string(ASCII 27 Esc)
  set(ColourReset "${Esc}[m")
  set(ColourBold  "${Esc}[1m")
  set(Red         "${Esc}[31m")
  set(Green       "${Esc}[32m")
  set(Yellow      "${Esc}[33m")
  set(Blue        "${Esc}[34m")
  set(Magenta     "${Esc}[35m")
  set(Cyan        "${Esc}[36m")
  set(White       "${Esc}[37m")
  set(BoldRed     "${Esc}[1;31m")
  set(BoldGreen   "${Esc}[1;32m")
  set(BoldYellow  "${Esc}[1;33m")
  set(BoldBlue    "${Esc}[1;34m")
  set(BoldMagenta "${Esc}[1;35m")
  set(BoldCyan    "${Esc}[1;36m")
  set(BoldWhite   "${Esc}[1;37m")
endif()

# Define project name
project(videostrip_project)
set(CMAKE_CXX_STANDARD 17)          # verify that the compiler supports C++17, as some targets could reversely fallback to c++14
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message ("${Cyan} Configuring system")
# GNU compiler flags. If you are using a different compiler, you may need to change these flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -O3 -Wformat")
# Additional flags for vectorized/multithread profiling and debugging
# Ask if compiler is GNU and support this flags, otherwise it could fail
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp -pthread -mavx2 -shared-libgcc")

# if (USE_CUDA)
#   message ("${BoldBlue} Asked for CUDA, searching... ${ColourReset}")
#   find_package(CUDA) # FindCUDA is deprecated, future releases will use find_package(CUDAToolkit)
#   if(CUDA_FOUND)
#     add_definitions( -DUSE_CUDA=1 )
#     include(FindCUDA)
#     message("${BoldGreen}" "CUDA library status: ${ColourReset}")
#     message(STATUS "    toolkit root dir: ${CUDA_TOOLKIT_ROOT_DIR}")
#     message(STATUS "    version: ${CUDA_VERSION}")
#     message(STATUS "    libraries: ${CUDA_LIBRARIES}")
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
add_executable(videostrip   src/videostrip-cli.cpp
                            src/helper.cpp
                            ${PROJECT_HEADERS})

############################ videostrip ####################
add_executable(videostrip-gui 
                            src/videostrip-gui.cpp
                            src/helper.cpp
                            src/gui/vsgui.cpp
                            ${IMGUI_SOURCES}
                            ${PROJECT_HEADERS})

target_compile_options(videostrip       PUBLIC -std=c++17 -mavx2) # the global flags may already have been set with std=c++17 
target_link_libraries (videostrip       ${CORE_LIBS} )

target_compile_options(videostrip-gui   PUBLIC -std=c++17 -mavx2) # the global flags may already have been set with std=c++17 
target_link_libraries (videostrip-gui   ${CORE_LIBS} ${GUI_LIBS} ${CMAKE_DL_LIBS} )
# target_link_libraries (videostrip-gui   ${CORE_LIBS} ${GUI_LIBS} ${CMAKE_DL_LIBS} glfw glut glu glfw3)

install (TARGETS videostrip             DESTINATION $ENV{HOME}/bin)
install (TARGETS videostrip-gui         DESTINATION $ENV{HOME}/bin)
# configure_file(scripts/tiff.convert2png.sh  $ENV{HOME}/bin/scripts)
