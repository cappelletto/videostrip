cmake_minimum_required(VERSION 3.18)

include(CTest)
enable_testing()

# ---- Catch2 (FetchContent) ----
include(FetchContent)
set(CATCH2_VERSION v3.5.3) # pin or bump as needed
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        ${CATCH2_VERSION}
)
FetchContent_MakeAvailable(Catch2)
include(Catch)  # provides catch_discover_tests()

# ---- Common includes for tests (allow <videostrip_core/...> etc.) ----
# Adjust if you need stricter scoping:
set(TEST_COMMON_INCLUDES
  ${CMAKE_SOURCE_DIR}
)

# =========================
# test_core_smoke
# =========================
add_executable(test_core_smoke
  ${CMAKE_CURRENT_SOURCE_DIR}/test_core_smoke.cpp
)
target_include_directories(test_core_smoke PRIVATE ${TEST_COMMON_INCLUDES})
target_link_libraries(test_core_smoke
  PRIVATE
    videostrip_core
    Catch2::Catch2WithMain
    ${OpenCV_LIBS}
)

catch_discover_tests(test_core_smoke
  WORKING_DIRECTORY $<TARGET_FILE_DIR:test_core_smoke>
)

# =========================
# test_config_yaml (optional)
# =========================
set(CONFIG_YAML_SRC ${CMAKE_CURRENT_SOURCE_DIR}/test_config_yaml.cpp)
if (EXISTS ${CONFIG_YAML_SRC})
  # Try both target and package styles for yaml-cpp (distro differences)
  set(YAMLCPP_LINK_TARGET "")
  if (TARGET yaml-cpp)
    set(YAMLCPP_LINK_TARGET yaml-cpp)
  else()
    find_package(yaml-cpp QUIET)
    if (TARGET yaml-cpp)
      set(YAMLCPP_LINK_TARGET yaml-cpp)
    else()
      # Some distros export 'YAML::yaml-cpp' imported target
      find_package(YAML-CPP QUIET)
      if (TARGET YAML::yaml-cpp)
        set(YAMLCPP_LINK_TARGET YAML::yaml-cpp)
      endif()
    endif()
  endif()

  if (YAMLCPP_LINK_TARGET)
    add_executable(test_config_yaml
      ${CONFIG_YAML_SRC}
      # Reuse the loader implementation from CLI module
      ${CMAKE_SOURCE_DIR}/videostrip_cli/config_loader.cpp
    )
    target_include_directories(test_config_yaml PRIVATE
      ${TEST_COMMON_INCLUDES}
      ${CMAKE_SOURCE_DIR}/videostrip_cli
    )
    target_link_libraries(test_config_yaml
      PRIVATE
        videostrip_core
        Catch2::Catch2WithMain
        ${YAMLCPP_LINK_TARGET}
    )

    # Only discover tests if the target exists (prevents the configure-time error)
    if (TARGET test_config_yaml)
      catch_discover_tests(test_config_yaml
        WORKING_DIRECTORY $<TARGET_FILE_DIR:test_config_yaml>
      )
      # Optional explicit CTest name, if you kept add_test() elsewhere:
      # add_test(NAME config_yaml COMMAND test_config_yaml)
    endif()
  else()
    message(WARNING "yaml-cpp not found; skipping test_config_yaml")
  endif()
endif()
