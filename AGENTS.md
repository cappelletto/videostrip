# videostrip - status & guide

_Last refreshed: 2025-10-17 12:13 UTC via `gh issue list --limit 20 --json number,title,state,labels,assignees`._

## Maintainers & Roles
- `@cappelletto` - project maintainer; currently driving performance profiling, exporter multithreading, and SIMD exploration across `videostrip_core` (#55, #56, #41, #42, #43).
- Unassigned work - issue backlog is open for contributors; see tracker snapshot below for entry points.

## Module Snapshot

### `videostrip_core/`
- C++17 frame extraction engine (OpenCV capture, keyframe selection, feature pipelines) plus metadata emitters.
- Current focus: throughput work (queue-based exporter, SIMD hot paths) and profiling instrumentation.

### `videostrip_cli/`
- CLI wrapper around `VideoFrameExtractor` with args.hxx parsing, YAML config, schema printing.
- Keep configs in `configs/`; use `--config` to point at presets.

### `videostrip_gui/`
- Experimental viewer; not built by default but kept for prototyping.

### `configs/`
- YAML presets demonstrating minimal and enhanced runs; safe starting point when debugging.

### `doc/`
- Generated manpages and supporting docs.

### `tests/`
- Catch2 suites (`test_core_smoke`, config/enhance coverage). Enable with `-DBUILD_TESTS=ON`; CI mirrors `tests/build_with_tests.sh`.

### `build/`
- CMake artifacts; ignored by git. Reconfigure with `cmake -S . -B build ...` as needed.

### `third_party/`
- Vendored libs (e.g. args.hxx) pinned for reproducible builds.

## Build & Test Checklist
- Configure: `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release`
- Build CLI: `cmake --build build --target videostrip_cli -j`
- Enable tests: add `-DBUILD_TESTS=ON`, then `cmake --build build --target test_core_smoke test_config_yaml -j`
- Run suites: `ctest --test-dir build --output-on-failure`

## Issue Tracker Snapshot (open issues)
| # | Title | Labels | Assignee |
|---|-------|--------|----------|
| #74 | Automated plugin self-test on Meshroom | - | Unassigned |
| #73 | Preflight binary check tool | - | Unassigned |
| #59 | Record enhancement & normalization config in summary.yaml | documentation | Unassigned |
| #58 | Performance regression smoke test in CI | performance | Unassigned |
| #57 | SIMD optimization for enhancement hot paths | performance | Unassigned |
| #56 | Queue-based Multithreaded Frame Export | performance | Unassigned |
| #55 | Profiling Report with VTune & perf | performance | Unassigned |
| #43 | Explore SIMD intrinsics acceleration for specific target-arch | enhancement | @cappelletto |
| #42 | Implement queue based multithreading write when exporting selected frame | enhancement | @cappelletto |
| #41 | Run in-depth VTune profiling and identify/prioritize optimization strategy | enhancement | @cappelletto |
| #32 | Extend frame selection overlap modes (Optical Flow & ECC) | enhancement | @cappelletto |

## Contribution Workflow
- Use 4-space indentation, Allman braces for types/namespaces, inline braces for control flow.
- Format with `clang-format -i` (LLVM style, 120 cols fallback).
- Public headers carry brief Doxygen docs.
- Commit style: `feat(core): add grid normalization`; link relevant issues (`Fixes #42`). Target PRs at `develop` and ensure `cmake --build build` plus `ctest` pass on Linux/Windows.
