add_library(videostrip_core STATIC
    videostrip_core.cpp
    helper.cpp
    headers.hpp
)

# Public headers live in this folder (P1204R0)
target_include_directories(videostrip_core
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
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
