cmake_minimum_required(VERSION 3.18)

include(FetchContent)
set(CATCH2_VERSION v3.5.3) # or latest stable

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        ${CATCH2_VERSION}
)
FetchContent_MakeAvailable(Catch2)

# ---- Core smoke test ----
add_executable(test_core_smoke
  test_core_smoke.cpp
)
target_link_libraries(test_core_smoke
  PRIVATE
    videostrip_core
    Catch2::Catch2WithMain
    ${OpenCV_LIBS}
)

# Allow <videostrip_core/...> and local includes in tests
target_include_directories(test_core_smoke PRIVATE ${CMAKE_SOURCE_DIR})

include(Catch)
catch_discover_tests(test_core_smoke)

# ---- Config YAML test ----
# Test: config YAML
add_executable(test_config_yaml
    test_config_yaml.cpp
    ${CMAKE_SOURCE_DIR}/videostrip_cli/config_loader.cpp  # reuse the loader impl
)

target_include_directories(test_config_yaml PRIVATE
    ${CMAKE_SOURCE_DIR}/videostrip_cli
)

target_link_libraries(test_config_yaml PRIVATE
    videostrip_core
    Catch2::Catch2WithMain
    yaml-cpp
)

add_test(NAME config_yaml COMMAND test_config_yaml)
# might be redundant with Catch2 auto-discovery, but no harm

include(Catch)
catch_discover_tests(test_config_yaml)
