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

include(CTest)
enable_testing()
include(Catch)

catch_discover_tests(test_core_smoke
  WORKING_DIRECTORY $<TARGET_FILE_DIR:test_core_smoke>)
catch_discover_tests(test_config_yaml
  WORKING_DIRECTORY $<TARGET_FILE_DIR:test_config_yaml>)

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

# Include packaging works data
include(CPack)
set(CPACK_PACKAGE_NAME "videostrip")
set(CPACK_PACKAGE_VENDOR "cappelletto")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_GENERATOR "ZIP;TGZ")