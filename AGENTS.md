# Repository Guide

## Module Overview
- `videostrip_core/` – C++17 library that implements the frame extraction engine, feature pipelines, metadata emitters, and logging.
- `videostrip_cli/` – Command-line entry point that wraps `VideoFrameExtractor`, handles argument/YAML config parsing, and exposes schema reporting.
- `videostrip_gui/` – Experimental viewer (not currently part of the default build).
- `configs/` – Sample YAML presets for CLI runs.
- `doc/` – Generated manpages and documentation artifacts.
- `tests/` – Catch2 suites plus helper scripts; enable with `-DBUILD_TESTS=ON`.
- `build/` – CMake build artifacts (ignored by git); `tests/build_with_tests.sh` mirrors CI configuration.
- `third_party/` – Vendored dependencies such as `args.hxx` for CLI parsing.

## videostrip_core Library
- `videostrip_core.hpp` / `.cpp` define `VideoFrameExtractor`, the main API that orchestrates OpenCV capture, feature extraction, optional enhancement, CSV/YAML output via `MetadataWriter`, and run summaries.
- `feature/feature_extractor.*` provides abstract `FeatureExtractor` plus a factory for ORB, AKAZE, SURF, and grid-based normalization hooks (#23). Normalization config lives in `FeatureNormalizationConfig`.
- `keyframe/keyframe_selector.*` exposes overlap and blur heuristics used to decide when to capture frames (ORB matching + RANSAC overlap, Laplacian variance sharpness).
- `io/metadata_writer.*` owns schema v1 output (`frames.csv`, `summary.yaml`) and paths derived from `ExtractorConfig`.
- `enhance/` contains YAML-driven enhancement definitions and the in-place `EnhanceStage` pipeline (depth/channel normalization, enhancer sequence execution).
- `logging/` implements a thread-safe `Logger` interface with `logger::ConsoleLogger`; integrate via `VideoFrameExtractor::setLogger`.
- `schema.hpp` keeps authoritative schema version constants plus YAML/JSON schema emitters used by CLI `--print-schema`.
- `helper.*`, `headers.hpp`, and `version.hpp.in` wrap legacy helpers, shared includes, and CMake-configured version stamping.

## videostrip_cli Application
- `videostrip_cli.cpp` wires `args.hxx` parsing, CLI overrides, optional YAML loading, version/schema reporting, and delegates to `VideoFrameExtractor`. It handles exit codes, progress logging, and error surfacing.
- `config_loader.*` reads YAML configs (via yaml-cpp), merges them into `ExtractorConfig`, normalizes output paths against the run directory, and propagates enhancement + normalization settings.
- `options.hpp` exposes a reusable parser setup for auxiliary tools, including global holders for `--version` and `--print-schema` states.
- `CMakeLists.txt` links against `videostrip_core`, adds yaml-cpp + OpenCV, and installs the `videostrip_cli` binary into `build/bin/`.
- Executable lives at `build/bin/videostrip_cli`; run with `./build/bin/videostrip_cli --config configs/sample_min.yaml`.

## Build, Test, and Development Commands
Configure once, then build incrementally:
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target videostrip_cli -j
```
Enable tests when needed:
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON
cmake --build build --target test_core_smoke test_config_yaml -j
ctest --test-dir build --output-on-failure
```
`tests/build_with_tests.sh` wraps the same flow. `BUILD_TESTS` controls Catch2 binaries such as `test_enhance_stage`.

## Testing Notes
- `tests/test_core_smoke.cpp` exercises `VideoFrameExtractor` end-to-end with temp directories and validates CSV/YAML schema v1 output.
- `tests/test_config_yaml.cpp` verifies YAML parsing, normalization, and override precedence for `ExtractorConfig`.
- `tests/test_enhance_yaml.cpp` and `tests/test_enhance_stage.cpp` cover enhance pipeline parsing/execution; `test_enhance_ops.cpp` focuses on individual operations.
- Each suite creates isolated temp directories; match their cleanup pattern when adding new tests.

## Coding Style & Conventions
Four-space indentation with Allman braces for namespaces/classes; inline braces for control flow. Types use `CamelCase`, methods `camelCase`, locals/files `snake_case`. Run `clang-format -i <file>` with the project config (fallback `-style=LLVM`, 120 columns). Document public headers with brief Doxygen comments.

## Contribution Workflow
Maintain the mixed Conventional Commit + short subject style (e.g., `feat(core): add grid normalization`). Reference GitHub issues (`Fixes #42`) when applicable. Target PRs at `develop` with a concise change log, screenshots/config examples for UX-facing work, and call out schema-impacting changes. Ensure `cmake --build build` and `ctest` succeed on Linux/Windows before requesting review.
