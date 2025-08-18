# Build the CLI executable
message(STATUS "*******************")
message(STATUS "Configuring module: videostrip_cli")
message(STATUS "*******************")

add_executable(videostrip_cli
    videostrip_cli.cpp
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
)