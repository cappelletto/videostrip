# Repository Guidelines

## Project Structure & Module Organization
`videostrip_core/` contains the C++17 extraction engine (feature detection, metadata writers, logging). `videostrip_cli/` builds the command-line entry point exported to `build/bin/videostrip_cli`. `videostrip_utils/` hosts shared helpers; `videostrip_gui/` is an experimental viewer. Pipeline templates for Meshroom live under `meshroom/`, sample YAML configs under `configs/`, and generated docs/manpages under `doc/`. Keep CMake build artifacts in `build/` and add new tests beside the existing Catch2 suites in `tests/`.

## Build, Test, and Development Commands
Configure once, then build incrementally:
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target videostrip_cli -j
```
Enable the test targets when you need them:
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON
cmake --build build --target test_core_smoke test_config_yaml -j
ctest --test-dir build --output-on-failure
```
`tests/build_with_tests.sh` wraps the same flow. Run the CLI locally with `./build/bin/videostrip_cli --config configs/sample_min.yaml`.

## Coding Style & Naming Conventions
Follow the existing spacing: four-space indents, Allman braces for namespaces/classes, and inline braces for control flow. Prefer `CamelCase` for types, `camelCase` for methods, and `snake_case` for local variables and files (as seen in `videostrip_core.cpp`). Run `clang-format -i <file>` with the project style; if the config is missing locally, fall back to `-style=LLVM` and keep 120-column lines. Document public headers with brief Doxygen blocks.

## Testing Guidelines
Unit tests rely on Catch2 and live in `tests/test_*.cpp`. Name new suites `test_<topic>.cpp` and tag them with `[core]`, `[cli]`, etc. Each test is expected to clean up temporary output directories—match the pattern in `test_core_smoke.cpp`. When touching schemas, add regression assertions to keep CSV/YAML outputs aligned with Schema v1. Prefer `ctest` for aggregated runs and call binaries (e.g., `./build/bin/test_config_yaml`) while debugging.

## Commit & Pull Request Guidelines
Git history mixes short descriptive subjects with Conventional Commit prefixes (`feat:`, `fix:`). Continue that style and reference GitHub issues (`Fixes #42`) when possible. Open PRs against `develop`, include a concise change log, configuration examples or screenshots for UX work, and note schema-impacting changes explicitly. CI runs on Linux and Windows, so ensure `cmake --build build` and `ctest` succeed before requesting review.
