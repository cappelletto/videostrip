# Contributing to videostrip

Thank you for considering contributing to **videostrip**!  
This document explains how to propose changes, what coding style to follow, and how your contributions will be licensed.

---

## Ways to Contribute

We welcome contributions of all types:

* **Bug reports**: File an issue with clear reproduction steps. Remember, the more context you provide the easier will be to identify the potential solution.
* **Feature requests**: Describe the scientific or technical motivation behind the request. Reference to articles, supporting diagrams, illustrations and similar are always welcomed.
* **Code contributions**: Either implement a new feature, fix a bug, or improve performance.
* **Documentation**: Improve README, companion user guides or illustrations or developer docs.
* **Testing**: Add or expand unit/integration tests.

---

## Development Workflow

1. **Fork & branch**  
   Fork the repository and create a new branch from `develop`.
   Regarding branch name convention, identify the related issue and use as part of the branch name: `feature/<issue-number>-<topic>` or `fix/<issue-number>-<topic>`. 
   For example: `feature/42-add-faster-than-light-performance`

2. **Build locally**  

```bash
   cmake -B build -DCMAKE_BUILD_TYPE=Release
   cmake --build build -j
   ctest --test-dir build
```

3. **Some coding guidelines**

* C++17 or later
* Prefer smart pointers, and std::optional over raw pointers.
* Prefer clang-format with the project’s style file (.clang-format).
* Use clang-tidy for static analysis and linting. A `.clang-tidy` configuration is provided to standardize checks.
* Add Doxygen-style comments for public headers.

4. **Add tests!**
New features or bugfixes should include a Catch2 unit test under `tests/`. You can use any of the existing tests as template or reference.

5. **Pull Request**

* Target the `develop` branch (which is the main active target).
* Include a description of changes and a reference to any related issue (e.g., Fixes #23).
* Use as reference `conventional commits`. See https://www.conventionalcommits.org/en/v1.0.0/
* Ensure code is well formatted (e.g., run `clang-format`).
* Run `clang-tidy` locally for static analysis. Pre-commit hooks are available to automate checks and ensure consistency.
* Make sure that CI (Linux + Windows) passes. You can try first building and testing locally and then Github Actions can be used for testing the rest of the OS-matrix

## Licensing of Contributions

This project uses a dual-licensing model:

* Apache License 2.0 — permissive license for broad adoption.
* GNU Lesser General Public License v3.0 (LGPL-3.0) — weak copyleft license ensuring modifications to the library itself remain open.

By submitting a contribution (via pull request, patch, or otherwise), you agree that your contribution is provided under both licenses. This means:

* Your code will be available to users under Apache-2.0 terms, *OR** under LGPL-3.0 terms, at their choice.
* You retain copyright to your contribution, but you grant us (and all users) a license under both terms.
* This dual licensing ensures flexibility for adopters while guaranteeing openness of core improvements.

If you cannot or do not wish to contribute under these terms, please do not submit patches directly — instead open an issue to discuss alternatives.
