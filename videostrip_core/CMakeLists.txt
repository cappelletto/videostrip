# videostrip_core/CMakeLists.txt
message(STATUS "*******************")
message(STATUS "Configuring module: videostrip_core")
message(STATUS "*******************")

add_library(videostrip_core STATIC
    videostrip_core.cpp
    feature/feature_extractor.cpp
    logging/logger.cpp
    logging/file_logger.hpp
)

# Public headers live in this folder (P1204R0)
target_include_directories(videostrip_core
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}       # so <videostrip_core/...> works
)

target_compile_definitions(videostrip_core
    PUBLIC GIT_COMMIT=\"${GIT_INFO}\"
)

# Link OpenCV to this module
target_link_libraries(videostrip_core
    PUBLIC ${OpenCV_LIBS}
)

# Optional: Warnings
if(MSVC)
    target_compile_options(videostrip_core PRIVATE /W4)
else()
    target_compile_options(videostrip_core PRIVATE -Wall -Wextra -pedantic)
endif()
