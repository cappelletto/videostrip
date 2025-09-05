# Build the CLI executable
message(STATUS "*******************")
message(STATUS "Configuring module: videostrip_cli")
message(STATUS "*******************")

find_package(yaml-cpp QUIET)
if(NOT yaml-cpp_FOUND)
  message(STATUS "yaml-cpp not found; fetching...")
  include(FetchContent)
  FetchContent_Declare(yaml_cpp GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git GIT_TAG 0.8.0)
  FetchContent_MakeAvailable(yaml_cpp)
endif()

add_executable(videostrip_cli
    videostrip_cli.cpp
    config_loader.cpp
)

# Include directories:
# - Current folder for any local headers (not needed now but safe)
# - Project external folder for args.hxx
# - Core module for headers.hpp and helper.hpp
target_include_directories(videostrip_cli
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/external
        ${CMAKE_SOURCE_DIR}/videostrip_core
)

# Link against the required modules and OpenCV
target_link_libraries(videostrip_cli
    PRIVATE
        videostrip_core
        ${OpenCV_LIBS}
        yaml-cpp
)