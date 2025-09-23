# Build the CLI executable
message(STATUS "*******************")
message(STATUS "Configuring module: videostrip_cli")
message(STATUS "*******************")

add_executable(videostrip_cli
    videostrip_cli.cpp
    config_loader.cpp
)


set_target_properties(videostrip_cli PROPERTIES
    LINK_SEARCH_START_STATIC ON
    LINK_SEARCH_END_STATIC ON
)

if(MSVC)
    target_link_options(videostrip_cli PRIVATE )
else()
    target_link_options(videostrip_cli PRIVATE -static-libstdc++ -static-libgcc)
endif()

# Include directories:
# - Current folder for any local headers (not needed now but safe)
# - Project external folder for args.hxx
# - Core module for headers.hpp and helper.hpp
target_include_directories(videostrip_cli
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/external
        ${CMAKE_SOURCE_DIR}/videostrip_core
)


set(_videostrip_cli_opencv_libs ${OpenCV_LIBS})
if(DEFINED OpenCV_3RDPARTY_LIBS AND OpenCV_3RDPARTY_LIBS)
    list(APPEND _videostrip_cli_opencv_libs ${OpenCV_3RDPARTY_LIBS})
endif()
list(REMOVE_DUPLICATES _videostrip_cli_opencv_libs)

# Link against the required modules and OpenCV
target_link_libraries(videostrip_cli
    PRIVATE
        videostrip_core
        yaml-cpp
        ${_videostrip_cli_opencv_libs}
)