# CMake file for tests (placeholder)
# Add test sources and link to core/utils as needed

add_executable(test_placeholder test_placeholder.cpp)
target_link_libraries(test_placeholder PRIVATE videostrip_core videostrip_utils)
add_test(NAME placeholder_test COMMAND test_placeholder)
