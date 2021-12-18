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
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -O3")
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

# Greedy include of all headers
file(GLOB PROJECT_HEADERS include/*.h include/*.hpp external/*.h external/*.hpp external/*.hxx)

# set (YAML_DIR $ENV{HOME}/local) # Path hint for YAML package installed from github

include_directories(BEFORE ../include
                    include
                    ${OpenCV_INCLUDE_DIRS}
                    )
          
# include_directories(AFTER ${YAML_DIR}/include)

# Retrieve git commit information, forward it as compiler flags so they can be used during building time
exec_program(
    "git"
    ${CMAKE_CURRENT_SOURCE_DIR}
    ARGS "describe --abbrev=4 --dirty --always --tags"
    OUTPUT_VARIABLE GIT_INFO )
add_definitions( -DGIT_COMMIT="${GIT_INFO}" ) # Forward current git info as a single string
add_definitions( -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}" ) # Forward current git info as a single string

                          ############################ MAD_TEST ####################
add_executable(videostrip #src/videostrip.cpp
                          src/main.cpp
                          src/helper.cpp
                          ${PROJECT_HEADERS})

# find_library (YAML_LIB yaml-cpp PATHS ${YAML_DIR}/lib64)      # for non-canonical installations of yaml library
# message (STATUS "Using yaml-cpp libraries from:" ${YAML_LIB})

target_compile_options(videostrip     PUBLIC -std=c++17 -mavx2) # the global flags may already have been set with std=c++17 
target_link_libraries(videostrip      ${OpenCV_LIBS} )

install (TARGETS videostrip   DESTINATION $ENV{HOME}/bin)
# configure_file(scripts/tiff.convert2png.sh  $ENV{HOME}/bin/scripts)
